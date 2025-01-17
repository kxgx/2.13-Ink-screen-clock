from flask import Flask, render_template, request, send_from_directory, redirect, url_for
import os
import re

app = Flask(__name__, template_folder='webui', static_url_path='', static_folder='webui')
FONT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'pic')  # 字体文件夹路径
MAIN_PY_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'bin', 'main.py')

def list_font_files(font_dir):
    try:
        return os.listdir(font_dir)
    except Exception as e:
        print(f"Error listing font files: {e}")
        return []  # 返回空列表以避免迭代错误

@app.route('/edit_main_py')
def edit_main_py():
    try:
        with open(MAIN_PY_PATH, 'r') as file:
            content = file.read()
    except Exception as e:
        return f"Error reading main.py: {e}", 500
    return render_template('edit_main_py.html', content=content)

@app.route('/save_main_py', methods=['POST'])
def save_main_py():
    new_content = request.form.get('content')
    if new_content is None:
        return "No content provided", 400
    try:
        with open(MAIN_PY_PATH, 'w') as file:
            file.write(new_content)
    except Exception as e:
        return f"Error saving main.py: {e}", 500
    return redirect(url_for('index'))

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

@app.route('/fonts/<filename>')
def fonts(filename):
    return send_from_directory(FONT_DIR, filename)

if not os.path.exists(FONT_DIR):
    os.makedirs(FONT_DIR)

if __name__ == '__main__':
    # 绑定到0.0.0.0，允许远程访问
    app.run(host='0.0.0.0', port=80, debug=False)
