import ctypes
import json
import sys
from pathlib import Path
import time

here = Path(__file__).resolve().parent
name = sys.argv[1] if len(sys.argv) > 1 else 'initial'
kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
class FileTime(ctypes.Structure):
    _fields_ = [('low', ctypes.c_uint32), ('high', ctypes.c_uint32)]
def times():
    idle, kern, user = FileTime(), FileTime(), FileTime()
    if not kernel32.GetSystemTimes(ctypes.byref(idle), ctypes.byref(kern), ctypes.byref(user)):
        raise OSError(ctypes.get_last_error())
    return tuple((x.high << 32) | x.low for x in (idle, kern, user))
samples = []
before = times()
for i in range(600):
    time.sleep(2)
    after = times()
    idle, kern, user = [x-y for x,y in zip(after,before)]
    total = kern + user
    samples.append({'utc':time.time(), 'system_percent':(total-idle)*100/total if total else None})
    before = after
    if i % 5 == 0:
        (here/f'cpu-monitor-{name}.json').write_text(json.dumps(samples,indent=2)+'\n',encoding='utf-8')
    try:
        if json.loads((here/f'results-{name}.json').read_text(encoding='utf-8'))['status'] != 'running': break
    except (ValueError, OSError, KeyError): pass
(here/f'cpu-monitor-{name}.json').write_text(json.dumps(samples,indent=2)+'\n',encoding='utf-8')
