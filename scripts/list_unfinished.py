import re
from pathlib import Path

t = Path("i18n/gitdesk_en.ts").read_text(encoding="utf-8")
out = []
for m in re.finditer(r"<message>(.*?)</message>", t, re.S):
    block = m.group(1)
    if 'type="unfinished"' not in block:
        continue
    sm = re.search(r"<source>(.*?)</source>", block, re.S)
    if sm:
        out.append(sm.group(1))

Path("i18n/unfinished_sources.txt").write_text("\n".join(out), encoding="utf-8")
print(len(out))
