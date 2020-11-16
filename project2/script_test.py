import os
import subprocess

all_test_cases = os.listdir("test-r")
count = 0
passed_count = 0
failed_count = 0
for test_case in all_test_cases:
    if test_case.endswith("spl"):
        test_case_name = test_case[:-4]
        result = subprocess.run(['bin/splc', f"test-r/{test_case}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        actual_out = result.stdout.decode()
        with open(f"test-r/{test_case_name}.a", "wb") as of:
            of.write(result.stdout)
        expected_out = open(f"test-r/{test_case_name}.out", "r").read()
        actual_out = actual_out.replace("\r", "\n")
        expected_out = expected_out.replace("\r", "\n")
        r = ""
        if actual_out == expected_out:
            r = "passed"
            passed_count += 1
        else:
            r = "failed"
            failed_count += 1
            print(f"diff test-r/{test_case_name}.a test-r/{test_case_name}.out")
        #print(f"#{count} {test_case_name}: {r}")
        count += 1
print(f"=================\npassed: {passed_count}\nfailed: {failed_count}\n")