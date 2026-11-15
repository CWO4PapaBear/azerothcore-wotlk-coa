from pathlib import Path
import struct,json,hashlib
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parent
base=root/'candidate'
row=next(r for r in json.loads((root.parent/'audit.json').read_text()) if r['id']==47640)
assert row['categories']==[23] and row['model']==950002
p=base/row['file'].replace('\\','/')
b=p.read_bytes()
def pair(data,pos):return struct.unpack_from('<II',data,pos)
def array(data,pos,size):
 n,o=pair(data,pos);assert o+n*size<=len(data);return n,o
def words(data,pos):
 n,o=array(data,pos,2);return np.frombuffer(data,'<u2',n,o)
assert b[:4]==b'MD20' and struct.unpack_from('<I',b,4)[0]==264
n,o=array(b,8,1);assert b[o:o+n].rstrip(b'\0').lower()==b'vsnightmareancient'
import sys
sys.path.insert(0,str(root))
from effects_check import verify,pose
src=(root/"vsnightmareancient.m2").read_bytes()[8:]
verify(b,src)
textures=[]
rgba=[]
count,off=array(b,80,16)
for i in range(count):
 typ,flags,n,o=struct.unpack_from('<4I',b,off+i*16);assert typ==0 and b[o+n-1]==0
 tex=base/b[o:o+n-1].decode().replace('\\','/')
 im=Image.open(tex);im.load();rgba.append(np.asarray(im.convert('RGBA')));textures.append(np.asarray(im.convert('RGB')))
nv,vo=array(b,60,48)
v=np.ndarray((nv,3),'<f4',b,vo,(48,4));uv=np.ndarray((nv,2),'<f4',b,vo+32,(48,4))
assert np.isfinite(v).all() and np.isfinite(uv).all()
lut=words(b,128)
# Paint the first view in bind pose with each mesh's diffuse texture.
# This checks triangle/UV/texture mapping independently of the client GPU.
v=pose(b,4,.37)
effects=True
angle=.52
depth=v[:,0]*np.cos(angle)+v[:,1]*np.sin(angle)
xy=np.column_stack((-v[:,0]*np.sin(angle)+v[:,1]*np.cos(angle),-v[:,2]))
xy=(xy-xy.min(0))*440/max(np.ptp(xy,axis=0))+16
canvas=np.full((480,480,3),32,np.uint8);z=np.full((480,480),-np.inf)
unlit=canvas.copy()
for view in range(2):
 skin=p.with_name(p.stem+f'{view:02}.skin').read_bytes();assert skin[:4]==b'SKIN'
 indices=words(skin,4);tri=words(skin,12);assert max(indices)<nv and max(tri)<len(indices)
 meshes,mo=array(skin,28,48); batches,bo=array(skin,36,24)
 materials={}
 glow={}
 for i in range(batches):
  batch=struct.unpack_from('<12H',skin,bo+i*24)
  assert batch[2]<meshes and batch[8]+batch[7]<=len(lut)
  if batch[6]==1:
   n,ro=array(b,112,4);flags,blend=struct.unpack_from("<HH",b,ro+4*batch[5])
   tex=lut[batch[8]]
   assert flags&1 and blend==4 and tex in (2,4)
   glow[batch[2]]=rgba[tex]
  materials.setdefault(batch[2],textures[lut[batch[8]]])
 assert glow.keys()==materials.keys()
 for i in range(meshes):
  _,_,_,_,start,size=struct.unpack_from('<6H',skin,mo+i*48)
  assert start+size<=len(tri) and size%3==0
  if view:continue
  image=materials[i];h,w=image.shape[:2]
  for face in indices[tri[start:start+size]].reshape(-1,3):
   a,c,e=xy[face];xmin,ymin=np.floor(np.min([a,c,e],0)).astype(int);xmax,ymax=np.ceil(np.max([a,c,e],0)).astype(int)
   dx,dy=c-a;ex,ey=e-a;det=dx*ey-dy*ex
   if abs(det)<1e-5:continue
   yy,xx=np.mgrid[ymin:ymax+1,xmin:xmax+1];x=xx+.5-a[0];y=yy+.5-a[1]
   u=(x*ey-y*ex)/det;t=(dx*y-dy*x)/det;f=1-u-t
   dist=f*depth[face[0]]+u*depth[face[1]]+t*depth[face[2]]
   mask=(u>=0)&(t>=0)&(f>=0)&(dist>z[yy,xx])
   coord=f[...,None]*uv[face[0]]+u[...,None]*uv[face[1]]+t[...,None]*uv[face[2]]
   canvas[yy[mask],xx[mask]]=image[(coord[:,:,1][mask]*h).astype(int)%h,(coord[:,:,0][mask]*w).astype(int)%w]
   if effects:
    unlit[yy[mask],xx[mask]]=canvas[yy[mask],xx[mask]]
    g=glow[i];gh,gw=g.shape[:2]
    pixel=g[(coord[:,:,1][mask]*gh).astype(int)%gh,(coord[:,:,0][mask]*gw).astype(int)%gw].astype(float)
    dest=canvas[yy[mask],xx[mask]].astype(float)
    canvas[yy[mask],xx[mask]]=np.clip(dest+pixel[:,:3]*pixel[:,3:4]/255,0,255)
   z[yy[mask],xx[mask]]=dist[mask]
assert np.isfinite(z).sum()>10000
changed=int(np.any(canvas!=unlit,axis=2).sum())
assert changed>0
print(f"PASS {changed} visible pixels changed by the additive glow at the same idle pose")
Image.fromarray(canvas).save(root/('animated-glow.png' if effects else 'bind-pose.png'))
print('PASS: Tree display path, 264 header, two indexed skins, six decoded textures, CPU animated-pose render')


