import pandas as pd
import os

assert(os.path.isfile('profiles.csv')), "Couldn't find profiles.csv"
df = pd.read_csv("profiles.csv")

try:
	df['max_step']
	max_step = True
	print("Found max step")
except:
	max_step = False
	print("Didn't find max step")
	
try:
	df['trans_time']
	trans_time = True
	print("Found transition time")
except:
	trans_time = False
	print("Didn't find transition time")

profile_count, speed_count = df.shape
speed_count -= (1 + max_step + trans_time)

print(f"Found {profile_count} profiles with {speed_count} speeds")

header = open("speed_control/profiles.h", "w")

header.write(f"#define PROFILE_COUNT {profile_count}\n")
header.write(f"#define SPEED_COUNT {speed_count}\n")

if trans_time:
    header.write("#define PER_PROFILE_TRANSITION\n")
    header.write("#define TRANS_TIME\n")

if max_step:
    assert(trans_time), "Trying to use max_step without trans time. This is not allowed!"
    header.write("#define MAX_STEP\n")




for idx, row in df.iterrows():
    assert(idx == row['profile_id']), "Profile ids not in order!"
    extra_str = ""
    if trans_time:
        extra_str += " + 2 /* trans_time */"
    if max_step:
        extra_str += " + 1 /* max_step */"
    header.write(f"const uint8_t profile_{idx}[SPEED_COUNT{extra_str}] = {{ ")
    for speed in range(speed_count):
        voltage = row[str(speed)]
        voltage = min(max(voltage, 0.0), 5.0)
        pwm = int(voltage * 255.0 / 5.0)
        header.write(f"{', ' if speed != 0 else ''}{pwm}")
    if trans_time:
	    trans_time_val = row['trans_time']
	    low_byte = int(trans_time_val) % 256
	    high_byte = int(trans_time_val / 256) % 256
	    header.write(f", {low_byte}, {high_byte}")
    if max_step:
	    header.write(f", {int(row['max_step'])}")
    header.write("};\n")
	

header.write("const uint8_t * profile_array[PROFILE_COUNT] = { \n")
for i in range(profile_count):
    header.write(f"\tprofile_{i}")
    if i != profile_count - 1:
        header.write(f",")
    header.write("\n")

header.write("};\n")

