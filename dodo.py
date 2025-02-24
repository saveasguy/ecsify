import os
from pathlib import Path


build_path_param = {
    "name": "build_path",
    "short": "b",
    "long": "build-path",
    "default": "build/Release",
}


def task_config():
    """configure cmake"""
    return {
        "actions": [
            "cmake -S . -B %(build_path)s -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
                -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake"
        ],
        "params": [build_path_param],
    }


def task_lint():
    """run C++ code linters and static analyzers"""
    def source_files():
        for root, _, files in os.walk("."):
            root_path = Path(root)
            for file in files:
                file_path = root_path / file
                if file_path.suffix in {".h", ".cc"}:
                    yield str(file_path)
        return
    sources = " ".join(source_files())
    return {
        "actions": [
            "clang-tidy -p %(build_path)s " + sources,
            "cpplint --root=.. " + sources,
            "clang-format --dry-run -Werror " + sources,
        ],
        "task_dep": ["config"],
        "params": [build_path_param]
    }


def task_build():
    """build the project"""
    return {
        "actions": ["cmake --build %(build_path)s --parallel"],
        "task_dep": ["config"],
        "params": [build_path_param]
    }
