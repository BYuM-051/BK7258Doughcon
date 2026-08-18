from PIL import Image
import os

input_dir = "./png"
#input_dir ="c:/neurosys/nurosys/app/src/main/res/drawable-v24/"

output_dir = "./jpg"
os.makedirs(output_dir, exist_ok=True)

# 설정값
TARGET_QUALITY = 80  # 4MB를 맞추기 위한 품질 설정 (50~70 권장)
SUB_SAMPLING = 1   # 4:2:2 포맷 설정 0=>4:4:4  2=>4:2:0
count =0
for filename in os.listdir(input_dir):
    if filename.lower().endswith(".png"):
        png_path = os.path.join(input_dir, filename)
        jpg_path = os.path.join(output_dir, filename.replace(".png", ".jpg"))
        temp_jpg = "temp_output.jpg"
        
        # 1. 원본 PNG 크기 확인
        png_size = os.path.getsize(png_path)
        
        # 2. JPG로 임시 변환 및 크기 측정
        with Image.open(png_path) as img:
            if img.mode in ("RGBA", "P"):
                img = img.convert("RGB")
            
            # 4:4:4(subsampling=0)로 임시 저장
            img.save(temp_jpg, "JPEG", subsampling=SUB_SAMPLING, quality=TARGET_QUALITY, optimize=True)
            jpg_size = os.path.getsize(temp_jpg)
            # os.replace(temp_jpg, jpg_path)

            
            # 3. 크기 비교 후 결정
          
                # JPG가 더 작으면 JPG 저장
            count+=1
            os.replace(temp_jpg, jpg_path)
            print(f"[{filename}] 변환: JPG {count} ({png_size} -> {jpg_size} bytes)")

print("\n모든 작업이 완료되었습니다.")