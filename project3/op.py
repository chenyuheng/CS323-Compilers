#!/usr/bin/env python
# encoding: utf-8
import sys

def reduce_dummy_label():
    path = sys.argv[1]
    path = path[:-3] + "ir"
    new_code = []
    label_dict = {}
    with open(path, "r") as f:
        last_label_flag = 0
        last_label = ""
        for row in f.readlines():
            if row.startswith("LABEL"):
                label = row[6:-3]
                if last_label_flag == 1:
                    label_dict[label] = last_label
                else:
                    last_label = label
                    last_label_flag = 1
                    new_code.append(row)
            else:
                last_label_flag = 0
                new_code.append(row)
    new_new_code = []
    for code in new_code:
        new_row = code
        for key in label_dict.keys():
            if key == code[-len(key) - 1:-1]:
                new_row = code.replace(key+"\n", label_dict[key]+"\n")
        new_new_code.append(new_row)
    with open(path, "w") as f:
        f.write("".join(new_new_code))

def main():
    print("started optimization...")
    reduce_dummy_label()

if __name__ == "__main__":
    main()
