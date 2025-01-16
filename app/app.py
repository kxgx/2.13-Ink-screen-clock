from flask import Flask, render_template, request, send_from_directory
import os
import re

app = Flask(__name__, template_folder='webui/templates', static_url_path='', static_folder='webui/static')
FONT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'pic')  # 字体文件夹路径

def list_font_files(font_dir):
    try:
        return os.listdir(font_dir)
    except Exception as e:
        print(f"Error listing font files: {e}")
        return []  # 返回空列表以避免迭代错误

def update_main_py_font_names(font_names):
    main_py_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'main.py')
    try:
        with open(main_py_path, 'r') as file:
            content = file.read()

        # 为每个字体变量定义正则表达式模式
        patterns = {
            'font01': re.compile(r"(?<=font01\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*20\s*\))"),
            'font02': re.compile(r"(?<=font02\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*15\s*\))"),
            'font03': re.compile(r"(?<=font03\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*38\s*\))"),
            'font04': re.compile(r"(?<=font04\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*10\s*\))"),
            'font05': re.compile(r"(?<=font05\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*12\s*\))"),
            'font06': re.compile(r"(?<=font06\s*=\s*ImageFont\.truetype\(\s*os\.path\.join\(picdir,\s*)'[^']+'(?=\s*,\s*13\s*\))"),
        }

        # 使用正则表达式替换字体文件名
        for font_var, font_name in font_names.items():
            pattern = patterns.get(font_var)
            if pattern:
                # 确保字体文件名被正确地转义
                safe_font_name = re.escape(font_name)
                content = pattern.sub(f"'{safe_font_name}'", content)

        with open(main_py_path, 'w') as file:
            file.write(content)
    except Exception as e:
        print(f"Error updating main.py: {e}")
        return False
    return True

@app.route('/')
def index():
    font_files = list_font_files(FONT_DIR)
    return render_template('index.html', font_files=font_files)

@app.route('/upload', methods=['POST'])
def upload():
    if 'font_file' not in request.files:
        return '没有文件部分'
    file = request.files['font_file']
    if file.filename == '':
        return '没有选择文件'
    if file:
        filename = os.path.join(FONT_DIR, file.filename)
        file.save(filename)
        return '文件已上传成功'

@app.route('/update_font_names')
def update_font_names():
    font_files = list_font_files(FONT_DIR)
    return render_template('update_font_names.html', font_files=font_files)

@app.route('/save_font_names', methods=['POST'])
def save_font_names():
    # 获取表单数据
    font_names = {
        'font01': request.form.get('font01'),
        'font02': request.form.get('font02'),
        'font03': request.form.get('font03'),
        'font04': request.form.get('font04'),
        'font05': request.form.get('font05'),
        'font06': request.form.get('font06'),
    }
    
    # 更新 main.py 中的字体文件名
    if update_main_py_font_names(font_names):
        return '字体文件名已保存'
    else:
        return '保存字体文件名时发生错误', 500

@app.route('/fonts/<filename>')
def fonts(filename):
    return send_from_directory(FONT_DIR, filename)

if not os.path.exists(FONT_DIR):
    os.makedirs(FONT_DIR)

if __name__ == '__main__':
    # 绑定到0.0.0.0，允许远程访问
    app.run(host='0.0.0.0', port=80, debug=False)