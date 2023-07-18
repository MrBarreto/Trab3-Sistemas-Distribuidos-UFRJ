def validate(var):
    f = open(var, "r")
    lines = f.readlines()
    requests = []
    grants = []
    releases = []

    for line in lines:
        if ("[R] Request" in line):
            requests.append(int(line.split("-")[1]))
            continue
        if ("[S] Grant" in line):
            if (len(grants) != len(releases)):
                print(line)
                print(len(grants))
                print(len(releases))
                raise Exception("Invalid log file: invalid grants and releases sequence")
            grants.append(int(line.split("-")[1]))
            continue
        if ("[R] Release" in line):
            if (len(releases) != len(grants) - 1):
                raise Exception("Invalid log file: invalid grants and releases sequence")
            releases.append(int(line.split("-")[1]))
            continue

    for i in range(len(requests)):
        if (requests[i] != grants[i] or grants[i] != releases[i]):
            raise Exception("Invalid log file: invalid grants and releases sequence")

    print("Log file was successfully validated")

logs = ["log2.txt", "log4.txt", "log8.txt", "log16.txt", "log32.txt", "log64.txt", "log128.txt"]
for archive in logs:
    print(f"Validating {archive}")
    validate(archive)