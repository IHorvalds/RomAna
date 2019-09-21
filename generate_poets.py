import os

def main():
    file = open("poets.txt", "r")
    for line in file:
        poet = line.strip()
        if poet:
            command = "./gui 3 " + poet
            os.system(command)
    
if __name__ == '__main__':
    main()
