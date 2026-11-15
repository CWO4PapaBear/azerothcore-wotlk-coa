from pathlib import Path
import struct as s
import numpy as np
root=Path(__file__).resolve().parent
def pair(b,o):return s.unpack_from('<II',b,o)
def pose(b,index,phase):
 seqs,so=pair(b,28)
 seen=set()
 while s.unpack_from('<I',b,so+64*index+12)[0]&64:
  target=s.unpack_from('<H',b,so+64*index+62)[0]
  if target==index:break
  assert index not in seen and target<seqs
  seen.add(index);index=target
 time=phase*s.unpack_from('<I',b,so+64*index+4)[0]
 nb,bo=pair(b,44);mat=[]
 def sample(o,rot,default):
  interp,glob=s.unpack_from('<Hh',b,o)
  ns,ts=pair(b,o+4); nv,vs=pair(b,o+12)
  assert ns==nv and interp in (0,1)
  if not ns:return default
  slot=0 if glob>=0 or ns==1 else index
  if slot>=ns:return default
  n,t=pair(b,ts+8*slot);m,v=pair(b,vs+8*slot);assert n==m
  if not n:return default
  times=np.frombuffer(b,'<u4',n,t)
  typ='<i2' if rot else '<f4'; dim=4 if rot else 3
  keys=np.frombuffer(b,typ,n*dim,v).reshape(n,dim)
  lo=max(0,int(np.searchsorted(times,time,side='right'))-1);hi=min(n-1,lo+1)
  a,c=keys[[lo,hi]].astype(float)
  if rot:
   def decode(q):return np.where(q<0,q+32768,q-32767)/32767
   a,c=decode(a),decode(c)
   if a@c<0:c=-c
  frac=np.clip((time-float(times[lo]))/max(1,float(times[hi])-float(times[lo])),0,1) if interp else 0
  out=a+(c-a)*frac
  return out/np.linalg.norm(out) if rot else out
 for bone in range(nb):
  o=bo+88*bone
  parent=s.unpack_from('<h',b,o+8)[0];assert -1<=parent<bone
  pivot=np.frombuffer(b,'<f4',3,o+76)
  trans=sample(o+16,False,np.zeros(3));x,y,z,w=sample(o+36,True,[0,0,0,1]);scale=sample(o+56,False,np.ones(3))
  r=np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],[2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],[2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])*scale
  local=np.eye(4);local[:3,:3]=r;local[:3,3]=trans+pivot-r@pivot
  mat.append(mat[parent]@local if parent>=0 else local)
 n,vo=pair(b,60)
 v=np.ndarray((n,3),'<f4',b,vo,(48,4));weight=np.ndarray((n,4),'u1',b,vo+12,(48,1))/255
 ids=np.ndarray((n,4),'u1',b,vo+16,(48,1))
 matrices=np.asarray(mat)[ids]
 return ((np.einsum('nkij,nj->nki',matrices[:,:,:3,:3],v)+matrices[:,:,:3,3])*weight[:,:,None]).sum(1)
def verify(b,src):
 animated=0
 for i in range(pair(src,28)[0]):
  samples=[]
  for phase in (0,.37,.73):
   actual=pose(b,i,phase);assert np.isfinite(actual).all()
   np.testing.assert_allclose(actual,pose(src,i,phase),atol=1e-6)
   samples.append(actual)
  animated+=int(any(abs(v-samples[0]).max()>.001 for v in samples[1:]))
 assert animated>0
 n,off=pair(b,296);assert n==3
 for i,(bone,lifetime,rate) in enumerate([(47,3.5,10),(60,1.5,8),(61,1.5,8)]):
  q=off+i*476
  assert s.unpack_from('<HH',b,q+20)==(bone,5)
  for field,expected in [(132,65*.04238648),(152,lifetime),(176,rate)]:
   n,t=pair(b,q+field+12);assert n==1
   m,o=pair(b,t);assert m==1 and abs(s.unpack_from('<f',b,o)[0]-expected)<1e-5
 print(f'PASS 32 sequence entries sampled against source; {animated} show motion; 3 fog emitter bones, rates, lifetimes and scalar gravity')
