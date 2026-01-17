from functools import reduce
import os
import subprocess as sp
import xml.etree.ElementTree as ET

# for test on server (Linux)
vitis_cmd = "/opt/Xilinx/Vitis_HLS/2021.1/bin/vitis_hls"
# for test on local (Windows)
# vitis_cmd = "your_vitis_path\\vitis_hls.bat"

# You may set where to store the logging information,
# and the commented line dismisses the information.
log_file = "test.log"
project_name = "lab1_PDE"
top_name = "jacobi"

#----- DO NOT modify the code below -----
cur_dir = os.getcwd()

def checkCSim():
  print("Check C-Simulation Result: ", end='', flush=True)
  csim_report_dir = os.path.join(cur_dir, project_name, "solution1", "csim", "report", f"{top_name}_csim.log")
  try:
    with open(csim_report_dir, "r") as report:
      result = report.readlines()[-2].rstrip()
      if result == "INFO: [SIM 1] CSim done with 0 errors.":
        return True
  except BaseException:
    return False

def linear(a, b, c, d, x):
  return min((x - a) / (b - a) * (d - c) + c, c)

def compute_score(result):
  check = lambda rule: all([a <= b for a, b in zip(result, rule)])
  ref_cycles = [10000000, 5500000, 1200000, 160000]
  ref_scores = [0, 60, 85, 100]
  scores = [0]
  if check([ref_cycles[0], 100, 100, 100, 100]):
    scores.append(linear(ref_cycles[1], ref_cycles[0], ref_scores[1], ref_scores[0], result[0]))
  if check([ref_cycles[1], 50, 80, 60, 80]):
    scores.append(linear(ref_cycles[2], ref_cycles[1], ref_scores[2], ref_scores[1], result[0]))
  if check([ref_cycles[2], 30, 60, 30, 60]):
    scores.append(linear(ref_cycles[3], ref_cycles[2], ref_scores[3], ref_scores[2], result[0]))
  if check([ref_cycles[3], 15, 50, 20, 50]):
    scores.append(100)
  return max(scores)

def print_score_message(score):
  if score >= 100:
    print('You are very close to the theoretical limit!')
    print('  Congratulations on your outstanding performance!')
    print('  Your commitment to HLS is truly admirable!')
    print('  I believe you have understood the principles and methods of hardware acceleration.')
  elif score >= 85:
    print('Ok, you are almost there, however, the final performance gap is not a easy trial to optimize!')
    print('  Adjust your parameters, or some axi options and optimize!')
  elif score >= 60:
    print('You are getting on the right way!')
    print('  How to ensure the loop are fully pipelined every cycle?')
  else:
    print('Some fundamental problem is in your program, try to fix them!')
    print('  Is there something wrong with the c-simulation?')
    print('  Is too many unroll or too wrong pragma?')

def scoreSynth():
  print("Check Synthesis Result: ")
  synth_report_dir = os.path.join(cur_dir, project_name, "solution1", "syn", "report", f"{top_name}_csynth.xml")
  try:
    tree = ET.parse(synth_report_dir)
  except FileNotFoundError:
    print('Can\'t found the report file (xml). Please check the logging information.' )
    exit(0)
  root = tree.getroot()
  result = []
  result.append(int(root.findtext('./PerformanceEstimates/SummaryOfOverallLatency/Best-caseLatency')))
  print("| Latency = ", result[0], end=' | ', flush=True)

  res_path = './AreaEstimates/Resources/'
  available_res_path = './AreaEstimates/AvailableResources/'
  items = ['BRAM_18K', 'DSP', 'FF', 'LUT']
  for item in items:
    res = int(root.findtext(res_path + item))
    available_res = int(root.findtext(available_res_path + item))
    result.append(res * 100 // available_res)
    print(item + ' = ' + str(result[-1]), end=' | ', flush=True)

  print('')
  score = compute_score(result)
  print(f"Your score is: {score:.0f}")
  print_score_message(score)

if __name__=="__main__":
  with open(log_file, 'w') as flog:
    print("Create Vitis HLS Project, Add Files, Run C-Simulation and Synthesis : ", end='', flush=True)

    try:
      sp.run([vitis_cmd, "script.tcl"], stdout=flog, stderr=flog, timeout=60)
    except sp.TimeoutExpired:
      print("Timeout!", "Score is 0. Come on!", sep='\n')
      exit(0)
    except sp.SubprocessError:
      print("Failed!", "Score is 0. Come on!", sep='\n')
      exit(0)
    else:
      print("Finished")

    if not checkCSim():
      print("Failed!", "Score is 0. Please check C-Simulation.", sep='\n')
      exit(0)
    else:
      print("Passed!")
      scoreSynth()
