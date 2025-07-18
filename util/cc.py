from flask import Flask, request, send_from_directory
import os

app = Flask(__name__)

CURRENT_DIR = os.getcwd()

@app.route('/<path:filepath>')
def serve_file(filepath):
    print(f"Serving file: {filepath} from: {CURRENT_DIR}")
    return send_from_directory(CURRENT_DIR, filepath)

@app.route('/', methods=['POST'])
def show_form_request():
    body = ""
    if request.form:
        body = list(request.form.keys())[0]

    request_line = f"{request.method} {request.path} {request.environ.get('SERVER_PROTOCOL')}"
    headers = "\n".join([f"{key}: {value}" for key, value in request.headers.items()])
    full_raw_request = f"{request_line}\n{headers}\n\n{body}"

    print("--------------------------------------")
    print(request.data)
    print("--------------------------------------")
    print("content:", request.form.to_dict())
    print("--------------------------------------")

    return 'OK'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1234)
