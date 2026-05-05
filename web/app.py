import subprocess
import os
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(BASE_DIR)
COMPILER_DIR = os.path.join(PROJECT_ROOT, "compiler")
DSLC_EXE = os.path.join(COMPILER_DIR, "dslc.exe")

@app.route('/')
def index():
    return render_template("index.html")

@app.route('/compile', methods=['POST'])
def compile_code():
    try:
        data = request.get_json()
        # Accept 'source' (frontend) or 'code'/'dsl' (legacy/backwards compatibility)
        source = data.get('source') or data.get('code') or data.get('dsl')

        if not source:
            return jsonify({"error": "No code received from frontend", "stage": "dsl_compile"})

        # 1. Save DSL source to temporary file in compiler dir to keep paths simple
        dsl_file = os.path.join(COMPILER_DIR, "input.dsl")
        with open(dsl_file, "w") as f:
            f.write(source)

        # 2. Run DSL Compiler
        # dslc <input> -o out.c -ast
        # This will produce out.c and out.exe in the COMPILER_DIR
        compile_process = subprocess.run(
            [DSLC_EXE, "input.dsl", "-o", "out.c", "-ast"],
            capture_output=True,
            text=True,
            cwd=COMPILER_DIR
        )

        # Extract AST from stderr
        ast_content = ""
        full_stderr = compile_process.stderr
        if "── AST ──────────────────────" in full_stderr:
            try:
                parts = full_stderr.split("── AST ──────────────────────")
                if len(parts) > 1:
                    ast_content = parts[1].split("─────────────────────────────")[0].strip()
            except:
                pass

        if compile_process.returncode != 0:
            return jsonify({
                "error": full_stderr or "DSL Compilation failed",
                "stage": "dsl_compile",
                "ast": ast_content
            })

        # 3. Read generated C code
        generated_c = ""
        c_file = os.path.join(COMPILER_DIR, "out.c")
        if os.path.exists(c_file):
            with open(c_file, "r") as f:
                generated_c = f.read()

        # 4. Run the produced EXE
        exe_path = os.path.join(COMPILER_DIR, "out.exe")
        if not os.path.exists(exe_path):
             return jsonify({
                "error": "Compiler failed to produce out.exe\n" + full_stderr,
                "stage": "gcc",
                "ast": ast_content,
                "generated_c": generated_c
            })

        run_process = subprocess.run(
            [exe_path],
            capture_output=True,
            text=True,
            cwd=COMPILER_DIR
        )

        return jsonify({
            "output": run_process.stdout + run_process.stderr,
            "ast": ast_content,
            "generated_c": generated_c,
            "stage": "success"
        })

    except Exception as e:
        return jsonify({"error": str(e), "stage": "internal_error"})

if __name__ == "__main__":
    app.run(debug=True, port=5000)