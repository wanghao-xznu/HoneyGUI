#!/usr/bin/env python

import sys
import git
import subprocess
import os
import re
import time
import argparse
import shutil
import importlib
from project_update import *
from build.sdk_build import *

honeyRepo_env = "HoneyRepo"
manifest_path_env = "manifest"
sub_git_group_env = 'subGitGroup'
sub_git_path_env = 'subGitPath'


honeyRepo_uitl_dir = os.path.join(os.environ.get(honeyRepo_env), "build", "utils")

print("repo home: ", os.environ[honeyRepo_env])
print("honeyRepo_uitl_dir", honeyRepo_uitl_dir)
print("jenkins base", os.path.join(os.environ[honeyRepo_env], "build", "jenkins"))
print("subGitPath", os.environ.get("subGitPath", ""))
print(os.path.join(os.environ[honeyRepo_env], "build", "jenkins"))

sys.path.append(os.path.join(os.environ[honeyRepo_env], "build", "jenkins"))
sys.path.append(honeyRepo_uitl_dir)

from repo_helper import *
from jenkins_base import JenkinsCheckBase
from keil_build import *

Keil_path = os.environ.get("Keil_Path")

def SDK_handler(module, submodule, manifest_path, repo_home, chip_type):
    print("Skip CI build")
    return True

    global Keil_path
    repo = git.Repo(search_parent_directories=True)
    print("\n================ build {} ====================\n".format(chip_type), flush=True)
    keil_builder = SDKBuild(manifest_path, repo_home, chip_type)
    print("call build {}".format(chip_type))
    if not keil_builder.build_all_keil_projects(all=True, fail_fast=True, keil_path=Keil_path):
        print("build {} fail".format(chip_type))
        return False
    #win32_sim scons
    os.chdir('./win32_sim')
    try:
        subprocess.check_call(["scons.exe"], universal_newlines=True, stderr=subprocess.STDOUT)
    except Exception as e:
        os.chdir('./..')
        print("win32_sim: 'scons' fail: {}".format(e))
        return False
    os.chdir('./..')
    #reset
    repo.git.checkout('--', '.')
    repo.git.clean('-dfx')
    #keil_sim scons --target=mdk5
    change_or_revert_macros(repo, "./keil_sim/menu_config.h", "change", [("BUILD_USING_SCRIPT_AS_A_APP", "", "BUILD_USING_SCRIPT_AS_A_APP")], True)
    os.chdir('./keil_sim')
    try:
        subprocess.check_call(["scons.exe", "--target=mdk5"], universal_newlines=True, stderr=subprocess.STDOUT)
    except Exception as e:
        os.chdir('./..')
        print("scons after enable BUILD_USING_SCRIPT_AS_A_APP fail: {}".format(e))
        return False
    os.chdir('./..')

    return True


def DOC_handler(module, submodule, manifest_path, repo_home, chip_type):
    print("Skip CI build")
    return True

    print("build sphinx document\n")
    try:
        cmd = ["python", os.path.join(os.path.dirname(os.path.abspath(__file__)), "../doc/source/build.py")]
        print(" ".join(cmd), flush=True)
        subprocess.check_call(cmd, universal_newlines=True, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print("Generate sphinx document exception: {}".format(e))
        return False
    return True


def GUI_handler(module, submodule, manifest_path, repo_home, chip_type):
    print("Skip CI build")
    return True

    ret1 = SDK_handler(module, submodule, manifest_path, repo_home, chip_type)
    if not ret1:
        print("SDK build fail!")
        return False
    ret2 = DOC_handler(module, submodule, manifest_path, repo_home, chip_type)
    if ret1 and ret2:
        return True
    else:
        return False



def App_handler(module, submodule, manifest_path, repo_home, chip_type):
    print("app handler")
    global Keil_path
    print("\n================ build {} ====================\n".format(chip_type), flush=True)
    keil_builder = SDKBuild(manifest_path, repo_home, chip_type)
    print("call build {}".format(chip_type))
    if not keil_builder.get_keyword_project_list(exact=True, folder=submodule, ignore_warning=True, error_record_path="", start_from_last_fail=True, only_build_app=False,keil_path=Keil_path):
        print("build {} fail".format(chip_type))
        return False

    return True


def Debug_handler(module, submodule, manifest_path, repo_home, chip_type):
    print('Bypass Module \'{}\''.format(module), flush=True)
    return False


def Keyword_handler(module, submodule, manifest_path, repo_home, chip_type):
    print("Keyword handler")
    global Keil_path
    print("\n================ build {} ====================\n".format(chip_type), flush=True)
    keil_builder = SDKBuild(manifest_path, repo_home, chip_type)
    print("call build {}".format(chip_type))
    if not keil_builder.get_keyword_project_list(exact=False, folder=submodule, ignore_warning=True, error_record_path="", start_from_last_fail=True, only_build_app=False,keil_path=Keil_path):
        print("build {} fail".format(chip_type))
        return False

    return True


# module tuple table: module, submodule, handler
module_table =  (
                    ['sdk', 'GUI',          '',         GUI_handler ],
                    ['sdk', 'Doc',          '',         DOC_handler ],

                    ['sdk', 'Script',       'CI',            SDK_handler ],
                    ['sdk', 'Script',       'Other',         SDK_handler ],
                    ['sdk', 'Script',       'Nightly',       SDK_handler ],
                    ['sdk', 'Script',       'Release',       SDK_handler ],

                    ['sdk', 'Debug',        '',       Debug_handler ],
            )


class CIBuild(JenkinsCheckBase):
    # subgit_repo_path: the project trigger the jenkins
    def __init__(self, subgit_repo_path, subgit_group_name, manifest_path, repo_home, **kwargs):
        self.subgit_repo_path = subgit_repo_path
        self.subgit_repo = git.Repo(path=subgit_repo_path)

        self.subgit_group_name = subgit_group_name
        self.manifest_path = manifest_path
        self.repo_home = repo_home
        self.chip_type = kwargs["chip_type"]


    def get_module_submodule_handler_from_commit_message(self, module_table):
        commit_title = self.subgit_repo.git.log('-1', 'HEAD', "--pretty='%s'").strip('\'')
        commit_head = commit_title.lower().split(':')[0].rstrip()
        sub_head = commit_head.split('-')[-1]
        if commit_head.split("-")[0] in ["app","keyword"]:
            commit_head = "{}-*".format(commit_head.split("-")[0])
        for (subgit_group_name, module, submodule, handler) in module_table:
            prefix = '-'.join([module, submodule]).rstrip('-')
            if commit_head == prefix.lower():
                if subgit_group_name == self.subgit_group_name:
                    return subgit_group_name, module, sub_head, handler
        return None, None, None, None


    def build_handler(self):
        commit_title = self.subgit_repo.git.log('-1', 'HEAD', "--pretty='%s'").strip('\'')
        group_name, module, submodule, handler = self.get_module_submodule_handler_from_commit_message(module_table)
        if not handler(module, submodule, self.manifest_path, self.repo_home, self.chip_type):
            self.gerrit_comment_wrap({}, label_name="Build-Verified", result=False, write_flag = True, message="Build project fail, refer to {}console for detail".format(os.environ['BUILD_URL']))
            return False
        return True


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='HoneyPatch CI build script')
    parser.add_argument('-c', '--chipType', help='Set the chip type')
    arg_dict = parser.parse_args()

    ci_build = CIBuild(subgit_repo_path=os.environ.get(sub_git_path_env), 
            subgit_group_name=os.environ.get(sub_git_group_env),
            manifest_path=os.environ.get(manifest_path_env),
            repo_home=os.environ.get(honeyRepo_env),
            chip_type = arg_dict.chipType)
    if not ci_build.build_handler():
        sys.exit(1)