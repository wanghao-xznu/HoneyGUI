import os
import sys
import argparse
import git
import subprocess
import ignore_check_file


honeyRepo_env = "HoneyRepo"
honeyRepo_uitl_dir = os.path.join(os.environ.get(honeyRepo_env), "build", "utils")

print("repo home: ", os.environ[honeyRepo_env])
print("honeyRepo_uitl_dir", honeyRepo_uitl_dir)
print("jenkins base", os.path.join(os.environ[honeyRepo_env], "build", "jenkins"))

sys.path.append(honeyRepo_uitl_dir)
sys.path.append(os.path.join(os.environ[honeyRepo_env], "build", "jenkins"))

from jenkins_base import JenkinsCheckBase
from jenkins_commit_version_check import CommitVersionCheckBase
from commit_feature_validate import *
from do_file_permission_check import *
from gerrit_comment_add_change import *


def sdk_handler(gitobj):
    pass

def debug_handler(gitobj):
    pass

def simulator_handler(gitobj):
    pass


class CICheck(JenkinsCheckBase):
    def __init__(self, subgit_repo_path, **kwargs):
        self.subgit_repo_path = subgit_repo_path
        self.subgit_repo = git.Repo(path=subgit_repo_path)

        self.module_table = (
                 [ 'GUI',        '',            kwargs["sdk_handler"] ],
                 [ 'DOC',        '',            kwargs["sdk_handler"] ],

                 [ 'Script',       'CI',        kwargs["sdk_handler"] ],
                 [ 'Script',       'Other',     kwargs["sdk_handler"] ],
                 [ 'Script',       'Nightly',   kwargs["sdk_handler"] ],
                 [ 'Script',       'Release',   kwargs["sdk_handler"] ]
        )


    def check_commit_valid(self):
        if not CommitVersionCheckBase().check_commit_valid(self.subgit_repo, os.environ['GERRIT_BRANCH'], os.environ['GERRIT_PATCHSET_REVISION']):
            return False, "This commit is too old, please do rebase"
        return True, ""


    def do_commit_check(self):
        self.subgit_repo.git.clean('-dfx')
        commit_validate_check_res, commit_validate_check_msg = self.check_commit_valid()
        if not commit_validate_check_res:
            return commit_validate_check_res,  self.COMMIT_CHECK_ERROR + " " + commit_validate_check_msg

        check_commit_message_res, check_commit_message_msg = self.check_commit_message(self.subgit_repo, self.module_table)
        if not check_commit_message_res:
            return check_commit_message_res, self.COMMIT_CHECK_ERROR + " " + check_commit_message_msg

        return check_commit_message_res, check_commit_message_msg



    def check_file_encodeing(self, changed_file_list, file_dir=None):
        ignore_file_list = ignore_check_file.ignore_file_encoding
        for f in changed_file_list:
            if any(ignore_file in f for ignore_file in ignore_file_list):
                print("Skip {}".format(f))
                continue
            if f.endswith(('.c', '.h', '.s', '.txt', '.html', '.md')):
                f = os.path.join(file_dir, f) if file_dir else f
                with open(f, 'rb') as fd:
                    for i, line in enumerate(fd, 1):
                        try:
                            line.decode('utf-8')
                            line.decode('gbk')
                            line.decode('cp950')
                        except UnicodeDecodeError as e:
                            return False, 'file {} encoding error {}, line {}, column {}'.format(
                                f, e.reason, i, e.start + 1)
        print("Check file encoding pass")
        return True, ""

    def get_changed_files(self, repo):
        ignore_file_list = ignore_check_file.ignore_file_check
        print("ignore_file_list", " ".join(ignore_file_list))
        commit_files = repo.git.log('-1', '--pretty=format:', '--name-status').strip('\'')
        if len(commit_files) > 0:
            commit_files = commit_files.split('\n')
        else:
            commit_files = []
        commit_files = list(filter(lambda x: x , commit_files))
    
        all_files_changed = list()
        all_files_changed_exclude_delete = list()
        for f in commit_files:
            if any(ignore_file in f for ignore_file in ignore_file_list):
                print(f"skip {f}")
                continue
            if f.startswith('A\t') or f.startswith('M\t'):
                all_files_changed.append(f.split('\t')[1])
                all_files_changed_exclude_delete.append(f.split('\t')[1])
            elif f.startswith('C') or f.startswith('R'):
                all_files_changed.append(f.split('\t')[2])
                all_files_changed_exclude_delete.append(f.split('\t')[2])
            elif f.startswith('D\t'):
                all_files_changed.append(f.split('\t')[1])

        return all_files_changed, all_files_changed_exclude_delete


    def check_commit_files(self):
        base_check_commit_files_res, base_check_commit_files_msg = super(CICheck, self).check_commit_files(self.subgit_repo, self.subgit_repo_path)
        if not base_check_commit_files_res:
            return base_check_commit_files_res, base_check_commit_files_msg

        all_files_changed, all_files_changed_exclude_delete = self.get_changed_files(self.subgit_repo)

        return True, ""


    def check_file_permission(self):
        commit_info = dict()
        permission_check_res, check_file_permitted_message, reviewer_check_message = do_file_permission_check(
                need_to_add_reviewer=False, commit_info=commit_info)

        if permission_check_res:
            return True, commit_info
        else:
            print_message = ""
            if check_file_permitted_message:
                print_message = check_file_permitted_message

            if reviewer_check_message:
                print_message += '\n[File Reviewer Check Error]: please refer to {}\n'.format(
                    get_funciton_owner_wiki_page_by_branch_name(os.environ['GERRIT_BRANCH']))

            print_message = print_message.lstrip().lstrip('\n')
            return False, print_message

    def check_commit_message(self, repo, module_table):
        # Commit string is capsulated in '', strip it
        commit_title = repo.git.log('-1', 'HEAD', "--pretty='%s'").strip('\'')
        commit_body = repo.git.log('-1', 'HEAD', "--pretty='%b'").strip('\'')
        commit_message = repo.git.log('-1', 'HEAD', "--pretty='%B'").strip('\'')
        commit_id = repo.git.log('-1', 'HEAD', "--pretty='%H'").strip("'")
        print(r'{}: {}'.format(commit_id, commit_title))

        # Check rule1: Separate subject from body with a blank line
        if commit_title != commit_message.split('\n', maxsplit=1)[0]:
            return False, 'no blank line between title and body'

        # Check rule2: Limit the subject line to 72 characters
        if len(commit_title) > 72 or len(commit_title) == 0:
            return False, 'invalid subject line len {} exceeds max 72 characters'.format(len(commit_title))

        # Check rule3: Capitalize first word of the subject line
        if commit_title.lstrip(' ')[0].islower():
            return False, 'first word \'{}\' of subject line not capitalized'.format(commit_title.split(maxsplit=1)[0])

        # Check rule4: Do not end the subject line with a period
        if commit_title.endswith('.'):
            return False, 'subject line should not end with a period'

        # Check rule5: Use the imperative mood in the subject line (!!!not checked)

        # Check rule6: Wrap the body at 72 characters
        for l in commit_body.split('\n'):
            if len(l) > 72:
                return False, 'body line \'{}\' len {} not wrapped at 72 characters'.format(l, len(l))

        # Check rule7: Use the body to explain what and why vs. how(!!!not checked)

        # Check rule8: Add module prefix for the subject line
        commit_head = commit_title.lower().split(':')[0].rstrip()
        main_head = commit_head.split('-')[0]
        sub_head = commit_head.split('-')[-1]
        if main_head in ["app", "keyword"]:
            commit_head = "{}-*".format(main_head)
        for (module, submodule, handler) in self.module_table:
            prefix = '-'.join([module, submodule]).rstrip('-')
            if commit_head == prefix.lower():
                if commit_title.lower().startswith(prefix.lower() + ': '):
                    break
                elif "*" in commit_head and commit_title.lower().replace(sub_head,'*').startswith(prefix.lower() + ': '):
                    break
                else:
                    return False, 'module name \'{}\' should suffixed with \': \' in subject line \'{}\''.format(prefix, commit_title)
        else:
            t = []
            for (m, s, _) in module_table:
                t.append('-'.join([m, s]).rstrip('-'))
            return False, 'no matched module name in subject line \'{}\' \n current supported modules {}'.format(commit_title, t)

        # Check rule9: Forbid reserved words in the subject line, e.g. jira
        if 'jira' in commit_title.lower():
            return False, 'illegal word \'jira\' in subject line'

        return True, ""


    def do_license_check(self):
        return True, ""


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='HoneyComb CI build script')
    parser.add_argument('-m', '--mode', default='CICheck', choices=[JenkinsCheckBase.STATIC_CHECK_STRATEGY_BEFORE_BUILD, JenkinsCheckBase.STATIC_CHECK_STRATEGY_AFTER_BUILD], help='Select CI static check mode')
    parser.add_argument('-c', '--chipType', help='Set the chip type')
    parser.add_argument('-r', '--repoPath', help='Set the subgit repo path')
    parser.add_argument('-t', '--TestRequest', action = 'store_true', help='Set the test-request label')

    arg_dict = parser.parse_args()
    ci_check = CICheck(subgit_repo_path=arg_dict.repoPath, 
                       sdk_handler=sdk_handler,
                       debug_handler=debug_handler,
                       simulator_handler=simulator_handler)

    label_map = {}
    if arg_dict.mode == JenkinsCheckBase.STATIC_CHECK_STRATEGY_BEFORE_BUILD:
        ci_check.gerrit_comment_wrap(label_map, label_name="Code-Review", result=False)
        #1.0 Check commit message
        #commit_check_res, commit_check_message = ci_check.do_commit_check()
        commit_check_res, commit_check_message = True, "By pass do_commit_check according to RD"
        if commit_check_res:
            ci_check.gerrit_comment_wrap(label_map, label_name="Commit-Check", result=True, message=None)
        else:
            print(commit_check_message)
            ci_check.gerrit_comment_wrap(label_map, label_name="Commit-Check", result=False, write_flag=True,
                                message=commit_check_message)
            sys.exit('check commit message failed')

        #2.0 check_commit_files
        #check_commit_files_result, check_commit_files_message = ci_check.check_commit_files()
        check_commit_files_result, check_commit_files_message = True, "By pass check_commit_files according to RD"
        if not check_commit_files_result:
            file_check_message = ci_check.STATIC_CHECK_ERROR + check_commit_files_message + '\n' + ci_check.WIKI_FOR_CI_CHECk
            ci_check.gerrit_comment_wrap(label_map, label_name="Static-Check", result=False, write_flag = True, message=file_check_message + '\n' + ci_check.BUILD_LOG_INFO)
            sys.exit('check commit files failed\n{}'.format(file_check_message))
        else:
            print("check commit file pass")
        #3.0 do_license_check
        do_license_check_res, do_license_msg = ci_check.do_license_check()
        if do_license_check_res:
            ci_check.gerrit_comment_wrap(label_map, label_name="License-Scan", result=True, write_flag=True, message='')
        else:
            ci_check.gerrit_comment_wrap(label_map, label_name="License-Scan", result=False, write_flag=True, message=do_license_msg + '\n' + ci_check.BUILD_LOG_INFO)
    elif arg_dict.mode == JenkinsCheckBase.STATIC_CHECK_STRATEGY_AFTER_BUILD:
        #check_file_permission, check_file_permission_msg = ci_check.check_file_permission()
        check_file_permission, check_file_permission_msg = True, "By pass check_file_permission according to RD"
        if not check_file_permission:
            # check_file_permission false
            file_check_message = ci_check.STATIC_CHECK_ERROR + check_file_permission_msg + '\n' + ci_check.WIKI_FOR_CI_CHECk
            ci_check.gerrit_comment_wrap(label_map, label_name="Static-Check", result=False, write_flag = True, message=file_check_message  + '\n' + ci_check.BUILD_LOG_INFO)
            sys.exit('check commit files failed, file permission error\n{}'.format(file_check_message))
        else:
            print("file permission check pass")
            ci_check.gerrit_comment_wrap(label_map, label_name="Static-Check", result=True)
            ci_check.gerrit_comment_wrap(label_map, label_name="Code-Review", result=True)
            ci_check.gerrit_comment_wrap(label_map, label_name="Test-Verified", result=True)
            #if check_file_permission_msg.get("Test-Verified", False):
            #    print("Test-Verified already set to 1, no need to test again")
            #else:
            #    ci_check.gerrit_comment_wrap(label_map, label_name="Test-Verified", result=True)

            ci_check.gerrit_comment_wrap(label_map, label_name=None, result=None, write_flag=True, message="No need to test")
