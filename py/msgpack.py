#!/usr/bin/python
# -*- coding: utf-8 -*-
import os


def main():
    for root, dirs, files in os.walk('../include/veigar/msgpack'):
        for f in files:
            file_path = os.path.join(root, f)
            if os.path.splitext(file_path)[-1] == '.hpp':
                macro_name = file_path.replace('include', '').replace('.hpp', '_hpp').replace('.', '').replace('\\', '_').replace('/', '_').upper()
                str_code = f"#ifndef {macro_name} // Add by msgpack.py\n#define {macro_name}\n"

                with open(file_path, 'r') as fp:
                    str_code += fp.read()
                    str_code += f"\n#endif // !{macro_name}"

                with open(file_path, 'w') as fp:
                    fp.write(str_code)
                print(f'{file_path}')

if __name__ == '__main__':
    main()
    print("Finished!")

