import collections,json,re,hashlib
from pathlib import Path
root=Path('.build/fast-io-throughput/inlining-review')
symbol='_ZN9blueberry9FastInputILb0EE4readIxEEbRT_'
rows={}
for name in ['baseline','current']:
 text=(root/(name+'.s')).read_text()
 begin=text.index('\n'+symbol+':\n')+1
 end=text.index('\n\t.size\t'+symbol+',',begin)
 block=text[begin:end]
 ins=[]
 for line in block.splitlines():
  match=re.match(r'\t([a-z][a-z0-9]*)\s*(.*?)\s*(?:#.*)?$',line)
  if match: ins.append((match[1],match[2].split('#',1)[0].strip()))
 rows[name]={'assembly_sha256':hashlib.sha256(text.encode()).hexdigest(),'read_i64_assembly_line':text[:begin].count('\n')+1,'static_instruction_count':len(ins),'conditional_branch_count':sum(op.startswith('j') and op!='jmp' for op,arg in ins),'call_targets':dict(collections.Counter(arg for op,arg in ins if op=='call')),'push_count':sum(op.startswith('push') for op,arg in ins),'stack_adjustments':[op+' '+arg for op,arg in ins if op in ['subq','addq'] and arg.endswith('%rsp')]}
 (root/(name+'-read-i64.s.txt')).write_text(block+'\n')
path=Path('C:/Users/kubar/Documents/GitHub/Blueberry-library/.verify-helper/cache/4eadb574dcb743b83b4a231fcab69002/test/digit_random_00.in')
lengths=collections.Counter()
with path.open() as src:
 count=int(next(src))
 for line in src:
  for token in line.split(): lengths[len(token.lstrip('+-'))]+=1
rows['digit_random_distribution']={'pairs':count,'tokens':sum(lengths.values()),'digits':dict(sorted(lengths.items())),'tokens_1_digit':lengths[1],'tokens_2_to_7_digits':sum(v for n,v in lengths.items() if 2<=n<=7),'tokens_8plus_digits':sum(v for n,v in lengths.items() if n>=8)}
(root/'analysis.json').write_text(json.dumps(rows,indent=2)+'\n')
print(json.dumps(rows,indent=2))
