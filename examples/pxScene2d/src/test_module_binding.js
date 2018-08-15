/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

print(uv_platform());
var hr = uv_hrtime()
print(hr[0] + " " + hr[1]);
print("0:"+uv_fs_access('C:\\windows\\notepad.exe', 'r'));
print("1:"+uv_fs_access('C:\\windows\\notepad1.exe', 'r'));
print("size:"+uv_fs_size('C:\\windows\\notepad.exe'));
var fd = uv_fs_open('C:\\windows\\notepad.exe', 'r', 420);
var buf = uv_fs_read(fd, 32, 0);
print(buf.byteLength+"");
uv_fs_close(fd);
var tm = uv_timer_new();
uv_timer_stop(tm);
var tm2 = uv_timer_new();
uv_timer_start(tm2, 1000, 1000, function () { print("OK2"); });
var tm1 = uv_timer_new();
uv_timer_start(tm1, 5000, 0, function () { print("OK1"); uv_timer_stop(tm2); });

uv_run_in_context("print(\"in eval context\");");

print("isV8 " + _isV8);
