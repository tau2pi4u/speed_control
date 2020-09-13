import pandas as pd
import os

assert(os.path.isfile('profiles.csv')), "Couldn't find profiles.csv"
df = pd.read_csv("profiles.csv")

profile_count, speed_count = df.shape
profile_count -= 1
speed_count -= 1

header = open("speed_control/profiles.h", "w")

header.write(f"#define PROFILE_COUNT {profile_count}\n")
header.write(f"#define SPEED_COUNT {speed_count}\n")

for idx, row in df.iterrows():
    assert(idx == row['profile_id']), "Profile ids not in order!"
    header.write(f"const uint8_t profile_{idx}[SPEED_COUNT] = {{ ")
    for speed in range(speed_count):
        voltage = row[str(speed)]
        voltage = min(max(voltage, 0.0), 5.0)
        pwm = int(voltage * 255.0 / 5.0)
        header.write(f"{', ' if speed != 0 else ''}{pwm}")
    header.write("};\n")

header.write("const uint8_t * profile_array[PROFILE_COUNT] = { \n")
for i in range(profile_count):
    header.write(f"\tprofile_{i}")
    if i != profile_count - 1:
        header.write(f",")
    header.write("\n")

header.write("};\n")

