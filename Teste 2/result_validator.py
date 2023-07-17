import re

def time_to_milliseconds(time_str):
    h, m, s, ms = map(int, time_str.split(':'))
    total_milliseconds = (h * 3600000) + (m * 60000) + (s * 1000) + ms
    return total_milliseconds

def extract_number_from_filename(filename):
    pattern = r"processos(\d+)\.txt"
    match = re.search(pattern, filename)

    if match:
        number = int(match.group(1))
        return number
    else:
        return None

def validate(archive):
    n = extract_number_from_filename(archive)
    r = 5
    f = open(archive, "r")
    pairs = []
    previous = 0
    lines = f.readlines()
    linecounter = 0
    for line in lines:
        splited = line.split()
        id = int(splited[2])
        hour_string = splited[-1]
        current = time_to_milliseconds(hour_string)
        if previous > current:
            raise Exception("Invalid result file: Time is decreasing")
        previous = current
        exists = False
        for i in range(len(pairs)):
            if pairs[i][0] == id:
                pairs[i][1] += 1
                exists = True
                break
        if (not exists):
            pairs.append([id, 1])
        linecounter += 1
    if linecounter != n*r:
        raise Exception("Invalid result file: insufucient lines")
    for i in range(len(pairs)):
        if pairs[i][1] != r:
            raise Exception("Invalid result file: insufucient lines by process")
    print("Result file was successfully validated")

processos = ["processos2.txt", "processos4.txt", "processos8.txt", "processos16.txt", "processos32.txt", "processos64.txt"]
for archive in processos:
    print(f"Validating {archive}")
    validate(archive)



        

