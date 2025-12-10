def plus(n):
    return '+' * n
def generate_char(char):
    number = int.from_bytes(char.encode(encoding='UTF-8',errors='strict'), "little")
    print(plus(number), "!0")
def generate_string(string : str):
    for char in string:
        generate_char(char)
generate_string("你好世界")