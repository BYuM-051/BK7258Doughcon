/* settings.c — All 87+ settings with defaults from SettingData.kt
 * Storage backend: Beken7258 NVS partition or flash area.
 * Replace bk_nvs_* calls with the actual SDK API on your target.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "lvgl.h"
#include "os/os.h"

#define TAG "[settings.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

/* [SAVE]/[LOAD] 로그 on/off: 아래 두 줄 중 하나만 활성화 */
//#define SAVE_LOG(fmt, ...) do {} while(0)
#define SAVE_LOG(fmt, ...) bk_printf(TAG fmt, ##__VA_ARGS__)

/* ---------------------------------------------------------------------------
 * Default values table (mirrors SettingData.kt settingArrayList)
 * ---------------------------------------------------------------------------*/
static setting_entry_t s_settings[] = {
    /* key                                  current  default */
    /* Basic */
    {"Mute",                                "0", "0"},
    {"LANGUAGE",                            "0", "0"},
    {"Degree",                              "\xc2\xb0""C", "\xc2\xb0""C"},  /* °C UTF-8 */
    {"DetailPassword",                      "71960", "71960"},
    {"NeurosysPassword",                    "0603", "0603"},
    /* Freeze */
    {"CurrentSaveFreezeTemp",               "-10", "-10"},
    {"CurrentSaveFreezeTimeHour",           "00", "00"},
    {"CurrentSaveFreezeTimeMin",            "01", "01"},
    /* Defrost */
    {"CurrentSaveDefreezeTemp",             "02", "02"},
    {"CurrentSaveDefreezeHumidity",         "", ""},
    {"CurrentSaveDefreezeTimeHour",         "03", "03"},
    {"CurrentSaveDefreezeTimeMin",          "00", "00"},
    /* Fermentation 1 */
    {"CurrentSaveFermentation1Temp",        "20", "20"},
    {"CurrentSaveFermentation1Humidity",    "70", "70"},
    {"CurrentSaveFermentation1TimeHour",    "03", "03"},
    {"CurrentSaveFermentation1TimeMin",     "00", "00"},
    /* Fermentation 2 */
    {"CurrentSaveFermentation2Temp",        "33", "33"},
    {"CurrentSaveFermentation2Humidity",    "80", "80"},
    {"CurrentSaveFermentation2TimeHour",    "02", "02"},
    {"CurrentSaveFermentation2TimeMin",     "00", "00"},
    /* Dry */
    {"CurrentSaveDryTemp",                  "35", "35"},
    {"CurrentSaveDryHumidity",              "20", "20"},
    {"CurrentSaveDryTimeHour",              "03", "03"},
    {"CurrentSaveDryTimeMin",               "00", "00"},
    /* Basic (factory reset values) */
    {"BasicCurrentSaveFreezeTemp",          "-10", "-10"},
    {"BasicCurrentSaveFreezeTimeHour",      "00", "00"},
    {"BasicCurrentSaveFreezeTimeMin",       "01", "01"},
    {"BasicCurrentSaveDefreezeTemp",        "02", "02"},
    {"BasicCurrentSaveDefreezeHumidity",    "", ""},
    {"BasicCurrentSaveDefreezeTimeHour",    "03", "03"},
    {"BasicCurrentSaveDefreezeTimeMin",     "00", "00"},
    {"BasicCurrentSaveFermentation1Temp",   "20", "20"},
    {"BasicCurrentSaveFermentation1Humidity","70","70"},
    {"BasicCurrentSaveFermentation1TimeHour","03","03"},
    {"BasicCurrentSaveFermentation1TimeMin","00", "00"},
    {"BasicCurrentSaveFermentation2Temp",   "33", "33"},
    {"BasicCurrentSaveFermentation2Humidity","80","80"},
    {"BasicCurrentSaveFermentation2TimeHour","02","02"},
    {"BasicCurrentSaveFermentation2TimeMin","00", "00"},
    {"BasicCurrentSaveDryTemp",             "35", "35"},
    {"BasicCurrentSaveDryHumidity",         "20", "20"},
    {"BasicCurrentSaveDryTimeHour",         "03", "03"},
    {"BasicCurrentSaveDryTimeMin",          "00", "00"},
    /* Manual mode */
    {"ManualFreezeTemp",                    "-15", "-15"},
    {"ManualDefrostTemp",                   "20", "20"},
    {"ManualFermentationTemp",              "20", "20"},
    {"ManualFermentationHumidity",          "80", "80"},
    /* RTC: Unix epoch saved to survive power cycle */
    {"rtc_epoch",                           "", ""},
    /* Memory slots 0-11 (15 items each) */
#define MEM_SLOT(N) \
    {"MemoryDayPeriod"#N,          "", ""}, \
    {"MemoryFreezeTemp"#N,         "", ""}, \
    {"MemoryDefrostTemp"#N,        "", ""}, \
    {"MemoryDefrostHour"#N,        "", ""}, \
    {"MemoryDefrostMin"#N,         "", ""}, \
    {"MemoryFermentation1Temp"#N,  "", ""}, \
    {"MemoryFermentation1Humidity"#N,"",""}, \
    {"MemoryFermentation1Hour"#N,  "", ""}, \
    {"MemoryFermentation1Min"#N,   "", ""}, \
    {"MemoryFermentation2Temp"#N,  "", ""}, \
    {"MemoryFermentation2Humidity"#N,"",""}, \
    {"MemoryFermentation2Hour"#N,  "", ""}, \
    {"MemoryFermentation2Min"#N,   "", ""}, \
    {"MemoryCompleteHour"#N,       "", ""}, \
    {"MemoryCompleteMin"#N,        "", ""},
    MEM_SLOT(0) MEM_SLOT(1) MEM_SLOT(2)  MEM_SLOT(3)
    MEM_SLOT(4) MEM_SLOT(5) MEM_SLOT(6)  MEM_SLOT(7)
    MEM_SLOT(8) MEM_SLOT(9) MEM_SLOT(10) MEM_SLOT(11)
    /* Record slots 0-4 (22 items each) */
#define REC_SLOT(N) \
    {"RecordFreezeTemp"#N,             "", ""}, \
    {"RecordFreezeTimeHour"#N,         "", ""}, \
    {"RecordFreezeTimeMin"#N,          "", ""}, \
    {"RecordDeFreezeTemp"#N,           "", ""}, \
    {"RecordDeFreezeTimeHour"#N,       "", ""}, \
    {"RecordDeFreezeTimeMin"#N,        "", ""}, \
    {"RecordFermentation1Temp"#N,      "", ""}, \
    {"RecordFermentation1Humidity"#N,  "", ""}, \
    {"RecordFermentation1TimeHour"#N,  "", ""}, \
    {"RecordFermentation1TimeMin"#N,   "", ""}, \
    {"RecordFermentation2Temp"#N,      "", ""}, \
    {"RecordFermentation2Humidity"#N,  "", ""}, \
    {"RecordFermentation2TimeHour"#N,  "", ""}, \
    {"RecordFermentation2TimeMin"#N,   "", ""}, \
    {"RecordStartMonth"#N,             "", ""}, \
    {"RecordStartDay"#N,               "", ""}, \
    {"RecordStartHour"#N,              "", ""}, \
    {"RecordStartMin"#N,               "", ""}, \
    {"RecordEndMonth"#N,               "", ""}, \
    {"RecordEndDay"#N,                 "", ""}, \
    {"RecordEndHour"#N,                "", ""}, \
    {"RecordEndMin"#N,                 "", ""},
    REC_SLOT(0) REC_SLOT(1) REC_SLOT(2) REC_SLOT(3) REC_SLOT(4)
    /* Detail settings */
    {"DetailDamperFanOn",               "", "4"},
    {"DetailDamperFanOff",              "", "2"},
    {"DetailDamperOffSol",              "", "120"},
    {"DetailDamperOnSol",               "", "120"},
    {"DetailDefrostOnOff",              "", "ON"},
    {"DetailDefrostReturnTemp",         "", "10"},
    {"DetailDefrostTime",               "", "6"},
    {"DetailHumidityOff",               "", "0"},
    {"DetailHumidityOd",                "", "3"},
    {"DetailHumidityRevision",          "", "0"},
    {"DetailTempOff",                   "", "0"},
    {"DetailTempOd",                    "", "1.5"},
    {"DetailFermentationTempOff",       "", "0"},
    {"DetailFermentationTempOd",        "", "3.0"},
    {"DetailTempRevision",              "", "0"},
    {"DetailFermentationTempRevision",  "", "0"},
    {"DetailHumidificationTime0",       "", "5"},
    {"DetailHumidificationTime1",       "", "0"},
    {"DetailWaterInterval0",            "", "2"},
    {"DetailWaterInterval1",            "", "0"},
    {"DetailHumidificationHeaterTime",  "", "30"},
    {"DetailOverFermentation",          "", "30"},
    {"DetailOverFermentationOnOff",     "", "OFF"},
    {"DetailFan",                       "", "OFF"},
    /* Operation state */
    {"saveChecking",                    "", "0"},
    {"saveOperationTemp",               "", "0"},
    {"saveDayPeriod",                   "", "0"},
    {"saveRemainHour",                  "", "0"},
    {"saveRemainMin",                   "", "0"},
    {"saveCurrentRemainHour",           "", "0"},
    {"saveCurrentRemainMin",            "", "0"},
    /* Power-off recovery */
    {"originYear",                      "", "2024"},
    {"originMonth",                     "", "3"},
    {"originDay",                       "", "29"},
    {"originHour",                      "", "7"},
    {"originMin",                       "", "00"},
    {"originCompleteYear",              "", "2024"},
    {"originCompleteMonth",             "", "3"},
    {"originCompleteDay",               "", "29"},
    {"originCompleteHour",              "", "7"},
    {"originCompleteMin",               "", "00"},
    {"CurrentCompleteYear",             "", "2026"},
    {"CurrentCompleteMonth",            "", "05"},
    {"CurrentCompleteDay",              "", "17"},
    {"CurrentCompleteHour",             "", "00"},
    {"CurrentCompleteMin",              "", "00"},
    {"Power",                           "", "0"},
    {"SaveWriting",                     "", "0"},
    {"CurrentSaveHour",                 "", "8"},
    {"CurrentSaveMin",                  "", "0"},
    {"settingday",                      "", "2026.01.01"},
    {"settingtime",                     "", "AM.  08 : 00"},
    {"SaveManual",                      "", "1"},
};

#define SETTINGS_TABLE_SIZE  (sizeof(s_settings) / sizeof(s_settings[0]))

/* ---------------------------------------------------------------------------
 * Internal: find entry by key
//  * ---------------------------------------------------------------------------*/
static setting_entry_t *_findorg(const char *key)
{
    bk_printf(TAG "%s", key);
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        if (strcmp(s_settings[i].key, key) == 0){
            bk_printf(TAG "[DEBUG] Value: %.2f\n", s_settings[i].value);            
            return &s_settings[i];
        }
    }
    return NULL;
}
static setting_entry_t *_find(const char *key)
{
    // 1. 검색 시작 알림 (줄바꿈 포함)
    // printf("[DEBUG] _find: Searching for [%s]\n", key);

    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        if (s_settings[i].key != NULL && strcmp(s_settings[i].key, key) == 0) {
            
            // 2. 값 결정 로직
            // value[0]이 '\0'이 아니면 value 사용, 비어있으면 default_value 사용
            const char* target_str = (s_settings[i].value[0] != '\0') ? 
                                      s_settings[i].value : s_settings[i].default_value;
            
            // 3. 문자열을 숫자로 변환 (숫자 표시 핵심)
            float numeric_val = (float)atof(target_str);
            
            // // 4. 결과 출력
            // printf("[DEBUG] Found! Index: %zu, String: '%s', Float: %.2f\n", 
            //         i, target_str, numeric_val);
            
            return &s_settings[i];
        }
    }

    // printf("[DEBUG] _find: Key [%s] not found.\n", key);
    return NULL;
}


/* Mutex protecting _ef_blob_save() against concurrent calls from lvgl and uart_comm tasks */
static beken_mutex_t s_ef_mutex = NULL;

/* ===========================================================================
 * BACKEND: Beken7258 NVS
 * Compile with -DBEKEN_TARGET to activate.
 * ===========================================================================*/
#ifndef HAL_USE_EMULATOR
#define BEKEN_TARGET
#endif
#ifdef BEKEN_TARGET

#include "bk_ef.h"

/* ---------------------------------------------------------------------------
 * EasyFlash blob storage — settings.dat content split into chunks.
 *
 * Layout in easyflash_ap (8K):
 *   "_sf_ct"   = "4"             ← number of chunks (compact index format)
 *   "_sf_00"   = "N=val\n..."    ← chunk 0: idx=value lines (≤ SF_CHUNK bytes)
 *   "_sf_01"   = ...             ← chunk 1
 *   ...
 *
 * Index format ("N=val\n"): avg 8 bytes/line vs 30 bytes for old "key=val\n".
 * 120 non-default entries → ~960 bytes → 3-4 chunks (old format: 9 chunks).
 * EasyFlash sector use: ~1900 bytes vs ~4000, leaving 2100 bytes free.
 * _ef_blob_load() supports both formats for backward compatibility.
 *
 * s_prev_chunk_count: last chunk count written (or loaded from flash).
 * Used to invalidate orphaned old-format chunks on first save after upgrade
 * (otherwise old _sf_04.._sf_08 remain valid and GC keeps moving them).
 * ---------------------------------------------------------------------------*/
#define SF_CHUNK   384    /* bytes per EasyFlash chunk value */

static int s_prev_chunk_count = 0;

static int _ef_blob_save(void)
{
    /* Sparse blob: only save entries where value differs from default_value.
     * Format: "%zu=%s\n" (index=value) — avg 8 bytes/line vs 30 bytes for
     * key=value.  120 non-default entries → ~960 bytes → 3-4 chunks (was 9).
     * EasyFlash sector use: ~1900 bytes vs ~4000 bytes, leaving 2100 bytes
     * free so GC triggers only every 4-5 saves instead of every chunk write.
     * _ef_blob_load() accepts both index and key formats (backward compat).
     *
     * Mutex: serializes concurrent calls from lvgl task and uart_comm task. */
    if (s_ef_mutex) rtos_lock_mutex(&s_ef_mutex);
    static char  buf[SF_CHUNK + 80];
    static char  ckey[12];
    int   bpos = 0, chunk = 0, skipped = 0, saved = 0;

    SAVE_LOG("[SAVE] ── flash write ────────────────────────────\n");
    int err = 0;
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        const char *v = s_settings[i].value;
        const char *d = s_settings[i].default_value ? s_settings[i].default_value : "";

        /* Skip if value == default — settings_init() will restore it correctly */
        if (strcmp(v, d) == 0) { skipped++; continue; }

        SAVE_LOG("[SAVE]   %-38s = \"%s\"\n", s_settings[i].key, v);

        char line[48];  /* max: "391=1781784669\n" = 18 chars; 48 is ample */
        int n = snprintf(line, sizeof(line), "%u=%s\n", (unsigned)i, v);
        if (n <= 0 || n >= (int)sizeof(line)) continue;

        /* Flush current chunk before adding a line that won't fit */
        if (bpos > 0 && bpos + n > SF_CHUNK) {
            buf[bpos] = '\0';
            snprintf(ckey, sizeof(ckey), "_sf_%02d", chunk++);
            {
                EfErrCode _rc = bk_set_env_enhance(ckey, buf, bpos + 1);
                if (_rc != EF_NO_ERR) {
                    SAVE_LOG("[SAVE] ERROR: bk_set_env_enhance(%s) rc=%d\n", ckey, (int)_rc);
                    err++;
                }
            }
            bpos = 0;
        }
        memcpy(buf + bpos, line, n);
        bpos += n;
        saved++;
    }
    /* Flush last chunk */
    if (bpos > 0) {
        buf[bpos] = '\0';
        snprintf(ckey, sizeof(ckey), "_sf_%02d", chunk++);
        EfErrCode _rc2 = bk_set_env_enhance(ckey, buf, bpos + 1);
        if (_rc2 != EF_NO_ERR) {
            SAVE_LOG("[SAVE] ERROR: bk_set_env_enhance(%s) rc=%d\n", ckey, (int)_rc2);
            err++;
        }
    }
    /* Write chunk-count sentinel — auto-saved by bk_set_env_enhance (v4) */
    snprintf(buf, sizeof(buf), "%d", chunk);
    {
        EfErrCode _rc3 = bk_set_env_enhance("_sf_ct", buf, (int)strlen(buf) + 1);
        if (_rc3 != EF_NO_ERR) {
            SAVE_LOG("[SAVE] ERROR: bk_set_env_enhance(_sf_ct) rc=%d\n", (int)_rc3);
            err++;
        }
    }
    /* Invalidate orphaned chunks left by previous (larger) format. */
    if (s_prev_chunk_count > chunk) {
        static const char _empty[1] = {'\0'};
        for (int ck = chunk; ck < s_prev_chunk_count; ck++) {
            snprintf(ckey, sizeof(ckey), "_sf_%02d", ck);
            bk_set_env_enhance(ckey, _empty, 1);
        }
        SAVE_LOG("[SAVE] invalidated %d orphaned chunks (_sf_%02d.._sf_%02d)\n",
               s_prev_chunk_count - chunk, chunk, s_prev_chunk_count - 1);
    }
    s_prev_chunk_count = chunk;

    /* NOTE: bk_save_env() must NOT be called here — it is the abandoned v3 API
     * and corrupts the v4 flash sector written by bk_set_env_enhance(). */
    SAVE_LOG("[SAVE] ── total: %d entries, %d chunks (skipped %d defaults)%s ──\n",
           saved, chunk, skipped, err ? " [ERRORS!]" : "");
    if (s_ef_mutex) rtos_unlock_mutex(&s_ef_mutex);
    return err;
}

static int _ef_blob_load(void)
{
    /* static: 385+12 = 397 bytes moved off the uart_comm task stack to BSS. */
    static char buf[SF_CHUNK + 1];
    static char ckey[12];

    /* Does a valid blob exist? */
    int ret = bk_get_env_enhance("_sf_ct", buf, sizeof(buf));
    if (ret <= 0) return 0;
    int nchunks = atoi(buf);
    if (nchunks > 32) return 0;
    /* nchunks == 0 is valid: blob was saved when all settings were at default.
     * Return 1 so _backend_load_all() treats it as "blob found, nothing to restore". */
    if (nchunks == 0) return 1;

    s_prev_chunk_count = nchunks;  /* remember for cleanup on next save */
    int loaded = 0, missing = 0;
    for (int c = 0; c < nchunks; c++) {
        snprintf(ckey, sizeof(ckey), "_sf_%02d", c);
        ret = bk_get_env_enhance(ckey, buf, sizeof(buf));
        if (ret <= 0) { missing++; bk_printf(TAG "[SETTINGS] chunk %s MISSING\n", ckey); continue; }

        char *p = buf;
        while (*p) {
            char *nl = strchr(p, '\n');
            if (nl) *nl = '\0';
            char *eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                /* Index format: "NNN=val" (digit-leading) — new compact format.
                 * Key format:   "KeyName=val"             — old format, kept for migration. */
                if (p[0] >= '0' && p[0] <= '9') {
                    unsigned idx = (unsigned)strtoul(p, NULL, 10);
                    if (idx < SETTINGS_TABLE_SIZE && s_settings[idx].dirty == 0) {
                        strncpy(s_settings[idx].value, eq + 1, sizeof(s_settings[idx].value) - 1);
                        s_settings[idx].value[sizeof(s_settings[idx].value) - 1] = '\0';
                        loaded++;
                    }
                } else {
                    setting_entry_t *e = _find(p);
                    if (e && e->dirty == 0) {
                        strncpy(e->value, eq + 1, sizeof(e->value) - 1);
                        e->value[sizeof(e->value) - 1] = '\0';
                        loaded++;
                    }
                }
                *eq = '=';   /* restore for safety */
            }
            if (nl) { p = nl + 1; } else { break; }
        }
    }
    if (missing > 0)
        bk_printf(TAG "[SETTINGS] WARNING: %d/%d chunks missing — EasyFlash may be full!\n",
               missing, nchunks);
    return loaded;
}

static void _backend_open(void)
{
    /* easyflash_init() is normally called by cli_easyflash_init() (CLI task),
     * but our background thread races against it and may win.
     * easyflash_init() is idempotent (guarded by s_g_easyflash_init_flag),
     * so calling it here is always safe and guarantees init_ok == true
     * before any bk_set/get_env_enhance() call. */
    EfErrCode rc = easyflash_init();
    if (rc != EF_NO_ERR)
        bk_printf(TAG "[SETTINGS] easyflash_init failed rc=%d\n", (int)rc);
    else
        bk_printf(TAG "[SETTINGS] easyflash_init OK\n");
}

static void _backend_load_all(void)
{
    uint32_t _t = lv_tick_get();

    /* Fast path: blob exists → read N chunks instead of 175 individual keys */
    int n = _ef_blob_load();
    if (n > 0) {
        bk_printf(TAG "[SETTINGS] blob load: %d keys in %lu ms\n", n, lv_tick_elaps(_t));
        return;
    }

    /* No blob yet — auto-create from current default values immediately.
     * Skips bk_get_env_enhance() loop (was ~27s due to WiFi-cal mutex). */
    bk_printf(TAG "[SETTINGS] no blob — creating from defaults\n");
    if (_ef_blob_save() != 0) {
        /* Partition full (EF_ENV_FULL=6) — old individual keys from previous firmware
         * occupy all space.  Erase the partition and retry once. */
        bk_printf(TAG "[SETTINGS] partition full — erasing EasyFlash and retrying\n");
        ef_env_set_default();
        _ef_blob_save();
    }
    bk_printf(TAG "[SETTINGS] blob created in %lu ms\n", lv_tick_elaps(_t));
}

/* Pending-save flag: set by settings_save_dirty() on the LVGL task,
 * consumed by settings_flush() on the uart_comm background task.
 * This prevents EasyFlash GC (130-150ms per cycle, 3-4 cycles per save)
 * from blocking the LVGL rendering task and causing 600-1800ms UI freezes. */
static volatile int s_save_pending = 0;

static void _backend_save_dirty(void)
{
    int any = 0;
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        if (s_settings[i].dirty) {
            s_settings[i].dirty = 0;
            any = 1;
        }
    }
    /* Defer the actual flash write: set flag and return immediately.
     * settings_flush() (called from uart_comm task every 200ms) performs
     * the real _ef_blob_save() on a background thread, not LVGL. */
    if (any) s_save_pending = 1;
}

static void _backend_commit(void) { /* bk_save_env() called inside _ef_blob_save */ }

/* ===========================================================================
 * BACKEND: PC file (./settings.dat)
 * Default when BEKEN_TARGET is not defined.
 * ===========================================================================*/
#else

#define SETTINGS_FILE     "./settings.dat"
#define SETTINGS_FILE_TMP "./settings.dat.tmp"

static void _backend_open(void) {}

static void _backend_load_all(void)
{
    FILE *f = fopen(SETTINGS_FILE, "r");
    if (!f) return;
    char line[96];
    while (fgets(line, sizeof(line), f)) {
        size_t n = strlen(line);
        while (n && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = '\0';
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        setting_entry_t *e = _find(line);
        if (e) {
            strncpy(e->value, eq + 1, sizeof(e->value) - 1);
            e->value[sizeof(e->value) - 1] = '\0';
        }
    }
    fclose(f);
}

static void _backend_save_dirty(void)
{
    FILE *f = fopen(SETTINGS_FILE_TMP, "w");
    if (!f) { bk_printf(TAG "[SETTINGS] fopen failed: %s\n", SETTINGS_FILE_TMP); return; }
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        fprintf(f, "%s=%s\n", s_settings[i].key, s_settings[i].value);
        s_settings[i].dirty = 0;
    }
    fclose(f);
    remove(SETTINGS_FILE);
    rename(SETTINGS_FILE_TMP, SETTINGS_FILE);
}

static void _backend_commit(void) {}

#endif /* BEKEN_TARGET */

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/
void settings_init(void)
{
    if (s_ef_mutex == NULL)
        rtos_init_mutex(&s_ef_mutex);
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        strncpy(s_settings[i].value, s_settings[i].default_value,
                sizeof(s_settings[i].value) - 1);
        s_settings[i].value[sizeof(s_settings[i].value) - 1] = '\0';
        s_settings[i].dirty = 0;
    }
    _backend_open();
    bk_printf(TAG "[INIT] %u entries set to default\n", (unsigned)SETTINGS_TABLE_SIZE);
}

static volatile int s_settings_loaded = 0;

int settings_is_loaded(void) { return s_settings_loaded; }

void settings_load_from_flash(void)
{
    _backend_load_all();
    s_settings_loaded = 1;

    /* Print every entry that differs from its default — these are the values
     * actually restored from flash (sparse blob only stores non-defaults). */
    int restored = 0;
    SAVE_LOG("[LOAD] ── restored from flash ─────────────────────\n");
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++) {
        const char *v = s_settings[i].value;
        const char *d = s_settings[i].default_value ? s_settings[i].default_value : "";
        if (strcmp(v, d) != 0) {
            SAVE_LOG("[LOAD]   %-38s = \"%s\"\n", s_settings[i].key, v);
            restored++;
        }
    }
    if (restored == 0)
        SAVE_LOG("[LOAD]   (all values are at default)\n");
    SAVE_LOG("[LOAD] ── total restored: %d entries ──────────────\n", restored);
}

void settings_save_all(void)
{
    for (size_t i = 0; i < SETTINGS_TABLE_SIZE; i++)
        s_settings[i].dirty = 1;
    _backend_save_dirty();
}

void settings_save_dirty(void)
{
    _backend_save_dirty();
}

void settings_flush(void)
{
    if (!s_save_pending) return;
    s_save_pending = 0;
    _ef_blob_save();
}

void settings_save_all_sync(void)
{
    /* Mark all dirty and write immediately (synchronous — used for explicit
     * user actions like memory slot save where a blocking save is acceptable).
     * Clears s_save_pending so the uart_comm background flush doesn't
     * double-write after this. */
    settings_save_all();  /* marks all dirty, calls _backend_save_dirty() → sets s_save_pending */
    s_save_pending = 0;   /* cancel deferred write — we write below */
    _ef_blob_save();      /* immediate synchronous write */
    _backend_commit();
}

void settings_save_all_reset(void)
{
    settings_save_all_sync();
}

/* 초기화(reset) 버튼이 되돌리는 대상: 기본설정 화면 5종(온도/습도/시간/댐퍼/제상설정)의
 * 값만 해당하며, detailsettingtemp_cb.c / detailsettinghumidity_cb.c / detailsettingtime_cb.c /
 * detailsettingdamper_cb.c / detailsettingdefrost_cb.c의 s_keys[] 배열과 정확히 일치한다.
 * 그 외(자동/수동/건조 운전값 CurrentSave*·Basic*, 메모리 불러오기 슬롯 Memory*, LANGUAGE,
 * Degree, DetailPassword 등)는 절대 건드리지 않는다 — 자동설정의 온도/습도/가동시간은
 * 마지막 운전정보를 그대로 기억해야 하고, 메모리 슬롯도 초기화 없이 남아있어야 한다. */
static const char * const s_factory_reset_keys[] = {
    /* 온도설정 (detailsettingtemp_cb.c s_keys) */
    "DetailTempOff", "DetailTempOd", "DetailFermentationTempOff", "DetailFermentationTempOd",
    "DetailTempRevision", "DetailFermentationTempRevision", "DetailFan",
    /* 습도설정 (detailsettinghumidity_cb.c s_keys) */
    "DetailHumidityOff", "DetailHumidityOd", "DetailHumidityRevision",
    /* 시간설정 (detailsettingtime_cb.c) */
    "DetailHumidificationTime0", "DetailHumidificationTime1",
    "DetailWaterInterval0", "DetailWaterInterval1",
    "DetailHumidificationHeaterTime",
    "DetailOverFermentationOnOff", "DetailOverFermentation",
    /* 댐퍼설정 (detailsettingdamper_cb.c s_keys) */
    "DetailDamperFanOn", "DetailDamperFanOff", "DetailDamperOnSol", "DetailDamperOffSol",
    /* 제상설정 (detailsettingdefrost_cb.c s_keys) */
    "DetailDefrostOnOff", "DetailDefrostReturnTemp", "DetailDefrostTime",
};
#define FACTORY_RESET_KEY_COUNT \
    (sizeof(s_factory_reset_keys) / sizeof(s_factory_reset_keys[0]))

void settings_factory_reset(void)
{
    /* 기본설정 5종 항목만 default_value로 되돌린다. 그 외 설정(자동/수동/건조 운전값,
     * 메모리 슬롯, 언어/온도단위, 비밀번호 등)은 전혀 건드리지 않는다. */
    for (size_t i = 0; i < FACTORY_RESET_KEY_COUNT; i++) {
        setting_entry_t *e = _find(s_factory_reset_keys[i]);
        if (!e) continue;
        strncpy(e->value, e->default_value, sizeof(e->value) - 1);
        e->value[sizeof(e->value) - 1] = '\0';
        e->dirty = 1;
    }

    _backend_save_dirty();  /* sets s_save_pending */
    settings_flush();       /* factory reset must write immediately */
    bk_printf(TAG "[SETTINGS] factory reset done (%u basic-setting keys reset; auto/manual run values, "
           "memory slots, language/degree preserved)\n", (unsigned)FACTORY_RESET_KEY_COUNT);
}

const char *settings_get_str0(const char *key)
{
    setting_entry_t *e = _find(key);
    return e ? e->value : "00";
}
const char *settings_get_str(const char *key)
{
    setting_entry_t *e = _find(key);
    
    if (e) {
        // 1. value[0]이 '\0'이면 비어있는 것이므로 default_value를 선택
        const char *val_to_return = (e->value[0] != '\0') ? e->value : e->default_value;
        
        // 2. 만약 default_value마저 NULL이라면 안전하게 "00" 반환
        if (val_to_return == NULL) val_to_return = "00";

        // printf("[DEBUG] settings_get_str: key[%s] -> value[%s]\n", key, val_to_return);
        return val_to_return;
    } 
    
    return "00";
}

int settings_get_int(const char *key)
{
    const char *v = settings_get_str(key);
    return v && *v ? (int)strtol(v, NULL, 10) : 0;
}

float settings_get_float(const char *key)
{
    const char *v = settings_get_str(key);
    return v && *v ? strtof(v, NULL) : 0.0f;
}

void settings_set_str(const char *key, const char *value)
{
    setting_entry_t *e = _find(key);
    if (e) {
        if (strncmp(e->value, value, sizeof(e->value)) != 0) {
            strncpy(e->value, value, sizeof(e->value) - 1);
            e->value[sizeof(e->value) - 1] = '\0';
            e->dirty = 1;
        }
    }
}

void settings_set_int(const char *key, int value)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    settings_set_str(key, buf);
}

void settings_set_float(const char *key, float value)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", value);
    settings_set_str(key, buf);
}

void settings_copy(const char *dst_key, const char *src_key)
{
    settings_set_str(dst_key, settings_get_str(src_key));
}
