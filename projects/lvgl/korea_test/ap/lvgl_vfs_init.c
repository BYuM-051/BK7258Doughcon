#include <stdlib.h>
#include "os/os.h"
#include "os/mem.h"
#include "os/str.h"
#include "bk_posix.h"

#if CONFIG_LITTLEFS
#include "driver/flash_partition.h"
#include "driver/flash.h"

#define SPI_FLASH_TEST 0
#define RENDER_DIRECT_MODE 1
#define FLASH_SECTOR 4096
#define LFS_PSRAM_ADDR (0x61000000 - CONFIG_USR_CONFIG_PARTITION_SIZE)
#define LFS_PSRAM_SIZE CONFIG_USR_CONFIG_PARTITION_SIZE

/* LFS_FLASH 타입으로 즉시 마운트 — PSRAM 복사 없음, ~1ms */
void lvgl_vfs_init_flash_quick(void)
{
    flash_protect_type_t type = bk_flash_get_protect_type();
    bk_flash_set_protect_type(FLASH_PROTECT_NONE);

    struct bk_little_fs_partition partition;
    partition.part_type = LFS_FLASH;
    partition.part_flash.start_addr = CONFIG_USR_CONFIG_PARTITION_OFFSET;
    partition.part_flash.size = CONFIG_USR_CONFIG_PARTITION_SIZE;
    partition.mount_path = "/";

    int ret = mount("SOURCE_NONE", "/", "littlefs", 0, &partition);
    if (ret < 0)
        bk_printf("[lvgl_vfs] quick flash mount fail:%d\r\n", ret);
        bk_printf("[lvgl_vfs] quick flash mount OK\r\n");

    bk_flash_set_protect_type(type);
}

/* flash→PSRAM 복사 후 LFS_MEM 으로 재마운트 (~12s, LVGL lock 상태에서 호출) */
void lvgl_vfs_copy_and_remount_psram(void)
{
    flash_protect_type_t type = bk_flash_get_protect_type();
    bk_flash_set_protect_type(FLASH_PROTECT_NONE);

    uint32_t addr = CONFIG_USR_CONFIG_PARTITION_OFFSET;
    uint32_t size;
    for (size = 0; size < CONFIG_USR_CONFIG_PARTITION_SIZE; size += FLASH_SECTOR) {
        bk_flash_read_bytes(addr + size, (uint8_t *)(LFS_PSRAM_ADDR + size), FLASH_SECTOR);
        if ((size & 0xFFFF) == 0)
            rtos_delay_milliseconds(20);
    }
    bk_flash_set_protect_type(type);

    umount("/");

    struct bk_little_fs_partition partition;
    partition.part_type = LFS_MEM;
    partition.part_flash.start_addr = LFS_PSRAM_ADDR;
    partition.part_flash.size = CONFIG_USR_CONFIG_PARTITION_SIZE;
    partition.mount_path = "/";

    int ret = mount("SOURCE_NONE", "/", "littlefs", 0, &partition);
    if (ret < 0)
        bk_printf("[lvgl_vfs] PSRAM remount fail:%d\r\n", ret);
    else
        bk_printf("[lvgl_vfs] PSRAM remount OK\r\n");
}

void lvgl_vfs_init_flash(void)
{
    bk_logic_partition_t *pt = bk_flash_partition_get_info(BK_PARTITION_USR_CONFIG);
    bk_printf("flash addr %x \n", pt->partition_start_addr);
    bk_printf("flash size %x \n", pt->partition_length);
   

#if 0 /*RENDER_PARTIAL_MODE*/
    struct bk_little_fs_partition partition;

    partition.part_type = LFS_FLASH;
    partition.part_flash.start_addr = CONFIG_USR_CONFIG_PARTITION_OFFSET;
#else
   void *test_psram = psram_malloc(4);

    bk_printf("LFS_PSRAM_ADDR=%x \n", LFS_PSRAM_ADDR);

    uint32_t addr = CONFIG_USR_CONFIG_PARTITION_OFFSET, size;
    for(size=0; size < CONFIG_USR_CONFIG_PARTITION_SIZE; size +=FLASH_SECTOR)
    {
        bk_flash_read_bytes(addr + size, (uint8_t *)(LFS_PSRAM_ADDR+size),  FLASH_SECTOR);
        if((size & 0xFFFF) == 0)
            rtos_delay_milliseconds(20);
    }
    struct bk_little_fs_partition partition;

    partition.part_type = LFS_MEM;
    partition.part_flash.start_addr = LFS_PSRAM_ADDR;
    os_free(test_psram);
#endif
    partition.part_flash.size = CONFIG_USR_CONFIG_PARTITION_SIZE;
    partition.mount_path = "/";
    char *fs_name = "littlefs";

    flash_protect_type_t type = bk_flash_get_protect_type();
    bk_flash_set_protect_type(FLASH_PROTECT_NONE);

    int ret = mount("SOURCE_NONE", partition.mount_path, fs_name, 0, &partition);
    if (ret < 0) {
        bk_printf("[%s][%d] mount[%s] fail:%d\r\n", __FUNCTION__, __LINE__, partition.mount_path, ret);
    } else {
        bk_printf("[%s][%d] mount[%s] success:%d\r\n", __FUNCTION__, __LINE__, partition.mount_path, ret);
    }
    bk_flash_set_protect_type(type);
}


void lvgl_vfs_init(void)
{
    lvgl_vfs_init_flash();
    // lvgl_vfs_init_spi_flash();
#if 0
    lvgl_vfs_init_qspi_flash();
    ap_vfs_init_usrdata_flash();

    int fd;
    #if 0
    fd = open("/res/img/img.txt", O_RDWR | O_CREAT | O_APPEND);
    if (fd < 0) {
        bk_printf("[%s][%d] open /res/img/img.txt fail:%d\r\n", __FUNCTION__, __LINE__, fd);
    } else {
        char test_buf[32];
        memset(test_buf, 0, sizeof(test_buf));
        int wr_size = read(fd, test_buf, sizeof(test_buf));
        bk_printf("[%s][%d] read /res/img/img.txt size=%d ret:%d\r\n", __FUNCTION__, __LINE__, sizeof(test_buf), wr_size);
        bk_printf("[%s]\r\n", test_buf);
        close(fd);
    }
    #endif
    
    #if 1
    fd = open("/usrdata/onoff.txt", O_RDWR | O_CREAT);
    if (fd < 0) {
        bk_printf("[%s][%d] open /usrdata/onoff.txt fail:%d\r\n", __FUNCTION__, __LINE__, fd);
    } else {
        #if 1
        lseek(fd, 0, SEEK_END);
        int file_len = ftell(fd);
        bk_printf("[%s][%d] file /usrdata/onoff.txt len:%d\r\n", __FUNCTION__, __LINE__, file_len);
        #endif

        char test_buf[32];
        int power_cnt = 0;

        lseek(fd, 0, SEEK_SET);
        os_memset(test_buf, 0, sizeof(test_buf));

        int wr_size = read(fd, test_buf, sizeof(test_buf));
        bk_printf("[%s][%d] read /usrdata/onoff.txt size=%d ret:%d\r\n", __FUNCTION__, __LINE__, sizeof(test_buf), wr_size);
        bk_printf("[%s]\r\n", test_buf);
        
        if(wr_size > 17) {
            power_cnt = atoi((const char *)&test_buf[17]);
            power_cnt++;
        } else {
            power_cnt = 1;
        }

        os_snprintf(test_buf, sizeof(test_buf), "sys power on cnt:%d", power_cnt);
        bk_printf("%s\r\n", test_buf);

        lseek(fd, 0, SEEK_SET);
        wr_size = write(fd, test_buf, sizeof(test_buf));
        bk_printf("[%s][%d] write /usrdata/onoff.txt size=%d ret:%d\r\n", __FUNCTION__, __LINE__, sizeof(test_buf), wr_size);
        
        close(fd);
    }
    #endif
#endif
}
#endif