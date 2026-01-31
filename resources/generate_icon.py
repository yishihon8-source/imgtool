"""
生成应用程序图标
需要安装 Pillow: pip install Pillow
"""
from PIL import Image, ImageDraw, ImageFont
import os

def create_icon():
    # 创建多个尺寸的图标
    sizes = [256, 128, 64, 48, 32, 16]
    images = []
    
    for size in sizes:
        # 创建图像
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # 绘制渐变背景圆形
        for i in range(size):
            alpha = int(255 * (1 - i / size))
            color1 = (66, 150, 250, 255)  # 蓝色
            color2 = (38, 100, 200, 255)  # 深蓝色
            
            # 线性插值
            t = i / size
            r = int(color1[0] * (1 - t) + color2[0] * t)
            g = int(color1[1] * (1 - t) + color2[1] * t)
            b = int(color1[2] * (1 - t) + color2[2] * t)
            
            draw.ellipse([i//4, i//4, size-i//4, size-i//4], 
                        fill=(r, g, b, 255))
        
        # 绘制字母 "P"
        font_size = int(size * 0.6)
        try:
            # 尝试使用系统字体
            font = ImageFont.truetype("arial.ttf", font_size)
        except:
            font = ImageFont.load_default()
        
        # 计算文字位置（居中）
        text = "P"
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        
        x = (size - text_width) // 2
        y = (size - text_height) // 2 - int(size * 0.05)
        
        # 绘制白色字母
        draw.text((x, y), text, fill=(255, 255, 255, 255), font=font)
        
        images.append(img)
    
    # 保存为 .ico 文件
    output_path = os.path.join(os.path.dirname(__file__), 'app.ico')
    images[0].save(output_path, format='ICO', sizes=[(s, s) for s in sizes])
    print(f"图标已生成: {output_path}")

if __name__ == '__main__':
    create_icon()





