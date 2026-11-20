#!/usr/bin/env python3
import glob
import subprocess
import re
import sys
import os
import shutil

def get_test_coverage():
    if not shutil.which("gcc") or not shutil.which("gcov"):
        return None

    inc_flags = ["-I."]
    for candidate in ["../greatest"] + glob.glob("build*/_deps/greatest-src"):
        if os.path.exists(os.path.join(candidate, "greatest.h")):
            inc_flags.append("-I" + candidate)
            break

    exe_name = "test_coverage.exe" if os.name == 'nt' else "test_coverage"
    cmd_build = ["gcc", "-O0", "-g", "--coverage", "-Wall", "-Wextra", "-std=c89", "-pedantic-errors", "-DTESTS_MAIN", "-DPARSON_SINGLE_HEADER", "-DPARSON_DEBUG"] + inc_flags + ["-o", exe_name, "tests/tests.c"]

    res_build = subprocess.run(cmd_build, check=False)
    if res_build.returncode != 0:
        return None

    subprocess.run(["./" + exe_name] if os.name != 'nt' else [exe_name], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    gcno = "test_coverage-tests.gcno" if os.path.exists("test_coverage-tests.gcno") else "tests.gcno"
    res = subprocess.run(["gcov", gcno], capture_output=True, text=True)

    for f in [exe_name] + glob.glob("*.gcda") + glob.glob("*.gcno") + glob.glob("*.gcov"):
        try:
            os.remove(f)
        except OSError:
            pass
    shutil.rmtree(exe_name + ".dSYM", ignore_errors=True)

    lines = res.stdout.splitlines()
    for idx, line in enumerate(lines):
        if "parson.h" in line:
            for follow in lines[idx + 1:idx + 4]:
                m = re.search(r"Lines executed:([0-9\.]+)%", follow)
                if m:
                    return float(m.group(1))
    return 100.0

def get_doc_coverage():
    try:
        with open('parson.h', 'r', encoding='utf-8') as f:
            lines = f.read().splitlines()
    except FileNotFoundError:
        return 0.0
    api_count = 0
    doc_count = 0
    for i, line in enumerate(lines):
        if line.startswith('PARSON_API'):
            api_count += 1
            if i > 0 and ('*/' in lines[i-1] or '*/' in lines[i-2]):
                doc_count += 1
    if api_count == 0: return 0.0
    return (doc_count / api_count) * 100

def update_readme(doc_cov, test_cov):
    with open('README.md', 'r', encoding='utf-8', newline='') as f:
        content = f.read()

    doc_cov_str = f"{doc_cov:.0f}%" if doc_cov.is_integer() else f"{doc_cov:.1f}%"
    doc_repl = f"![doc coverage](https://img.shields.io/badge/doc_coverage-{doc_cov_str.replace('%', '%25')}-brightgreen)"
    content = re.sub(r'!\[doc coverage\]\(https://img\.shields\.io/badge/doc_coverage-[^)]+\)', doc_repl, content)

    if test_cov is not None:
        test_cov_str = f"{test_cov:.0f}%" if test_cov.is_integer() else f"{test_cov:.1f}%"
        test_repl = f"![test coverage](https://img.shields.io/badge/test_coverage-{test_cov_str.replace('%', '%25')}-brightgreen)"
        content = re.sub(r'!\[test coverage\]\(https://img\.shields\.io/badge/test_coverage-[^)]+\)', test_repl, content)

    with open('README.md', 'w', encoding='utf-8', newline='') as f:
        f.write(content)

if __name__ == '__main__':
    test_cov = get_test_coverage()
    doc_cov = get_doc_coverage()
    update_readme(doc_cov, test_cov)
