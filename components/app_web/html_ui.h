const char index_html[] = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>SmartButton</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,sans-serif;background:#1a1a2e;color:#eee;min-height:100vh}
.header{background:#16213e;padding:12px 16px;text-align:center;border-bottom:1px solid #0f3460;position:sticky;top:0;z-index:50}
.header h1{font-size:1.15em;color:#00d4ff;margin:0}
.nav{position:fixed;bottom:0;left:0;right:0;background:#16213e;border-top:1px solid #0f3460;display:flex;z-index:50;padding-bottom:env(safe-area-inset-bottom)}
.nav-item{flex:1;text-align:center;padding:8px 0 6px;cursor:pointer;color:#556;font-size:.68em;transition:color .2s}
.nav-item .ico{font-size:1.4em;display:block;margin-bottom:2px}
.nav-item.act{color:#00d4ff}
.sec{display:none;padding:16px;padding-bottom:80px}
.sec.act{display:block}
.card{background:#16213e;border-radius:12px;padding:16px;margin-bottom:14px}
.card h2{font-size:1em;margin-bottom:12px;color:#00d4ff;border-bottom:1px solid #0f3460;padding-bottom:8px}
label{display:block;font-size:.85em;color:#aaa;margin-bottom:4px}
input,select{width:100%;padding:10px;border:1px solid #0f3460;border-radius:8px;background:#0f3460;color:#eee;font-size:.95em;margin-bottom:10px;-webkit-appearance:none}
input:focus,select:focus{outline:none;border-color:#00d4ff}
select{cursor:pointer}
.pw{position:relative}
.pw input{padding-right:44px}
.pw-tog{position:absolute;right:8px;top:6px;background:none;border:none;color:#aaa;font-size:1.2em;cursor:pointer;padding:4px;width:auto}
.wifi-list{max-height:200px;overflow-y:auto;margin-bottom:10px}
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:10px;border-radius:8px;cursor:pointer;margin-bottom:4px;background:#0f3460}
.wifi-item:hover,.wifi-item.sel{background:#1a3a6e;border:1px solid #00d4ff}
.wifi-item .name{font-size:.9em;flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.wifi-item .rssi{font-size:.75em;color:#aaa;margin-left:8px;white-space:nowrap}
.wifi-item .lock{font-size:.75em;margin-left:4px}
button,.btn{width:100%;padding:12px;border:none;border-radius:8px;font-size:.95em;font-weight:bold;cursor:pointer;color:#fff;margin-top:4px}
.btn-p{background:#0f3460;color:#00d4ff}
.btn-ok{background:#00d4ff;color:#1a1a2e}
.btn-ok:active{background:#00a8cc}
.btn-test{background:#0f3460;color:#e8a838;margin-top:6px}
.btn-danger{background:#e74c3c;color:#fff}
.btn-off{background:#333;color:#aaa;margin-top:8px}
.msg{text-align:center;padding:8px;border-radius:8px;margin-top:8px;font-size:.85em;display:none}
.msg.ok{display:block;background:#0a3d2a;color:#4ade80}
.msg.err{display:block;background:#3d0a0a;color:#f87171}
.pl-row{display:none}
.pl-row.show{display:block}
.tabs{display:flex;gap:6px;margin-bottom:10px}
.tab{flex:1;padding:8px;text-align:center;border-radius:8px;background:#0f3460;color:#aaa;cursor:pointer;font-size:.85em}
.tab.act{background:#1a3a6e;color:#00d4ff;border:1px solid #00d4ff}
.r2{display:flex;gap:8px}
.r2>div{flex:1}
.hid{display:none}
.login-box{max-width:320px;margin:60px auto 0;padding:16px}
.login-box h1{margin-bottom:24px;text-align:center;color:#00d4ff}
input[type="file"]{padding:6px;cursor:pointer}
.modal-bg{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.7);z-index:100;align-items:center;justify-content:center}
.modal-bg.show{display:flex}
.modal{background:#16213e;border-radius:12px;padding:20px;max-width:360px;width:90%;max-height:80vh;overflow-y:auto}
.modal h2{font-size:1em;color:#00d4ff;margin-bottom:14px;text-align:center}
.irow{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #0f3460;font-size:.85em}
.irow .il{color:#aaa}
.irow .iv{color:#eee;font-family:monospace}
.cgrid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-bottom:14px}
.cbtn{aspect-ratio:1;border-radius:12px;border:2px solid transparent;cursor:pointer;transition:border-color .2s,transform .1s}
.cbtn:active{transform:scale(.9)}
.cbtn.act{border-color:#fff}
.srow{margin-bottom:8px}
.srow label{display:flex;justify-content:space-between}
.srow input[type=range]{width:100%;margin-bottom:4px;-webkit-appearance:auto;background:transparent}
.lprev{width:60px;height:60px;border-radius:50%;margin:0 auto 14px;border:2px solid #333;transition:background-color .2s}
</style>
</head><body>

<div id="login" class="login-box">
<h1>🔐 SmartButton</h1>
<div class="card">
<label>Usuario</label>
<input id="luser" placeholder="admin" value="admin">
<label>Contraseña</label>
<div class="pw">
<input id="lpass" type="password" placeholder="Contraseña">
<button class="pw-tog" type="button" onclick="togVis('lpass')">🗝️</button>
</div>
<button class="btn btn-ok" onclick="doLogin()">Entrar</button>
<div id="lmsg" class="msg"></div>
</div>
</div>

<div id="app" class="hid">
<div class="header"><h1>⚡ SmartButton</h1></div>

<!-- WiFi -->
<div id="s-wifi" class="sec act">
<div class="card">
<h2>📶 WiFi</h2>
<button class="btn btn-p" onclick="scanWifi()">Buscar redes</button>
<div id="wlist" class="wifi-list"></div>
<label>SSID</label>
<input id="ssid" placeholder="Nombre de la red">
<label>Password</label>
<div class="pw">
<input id="wpass" type="password" placeholder="Contraseña WiFi">
<button class="pw-tog" type="button" onclick="togVis('wpass')">🗝️</button>
</div>
<button class="btn btn-ok" onclick="saveWifi()">Guardar WiFi</button>
<div id="wmsg" class="msg"></div>
</div>
</div>

<!-- Botones -->
<div id="s-btn" class="sec">
<div class="card">
<h2>🔘 Botones</h2>
<div class="tabs">
<div class="tab act" onclick="swTab(1,this)">Boton 1</div>
<div class="tab" onclick="swTab(2,this)">Boton 2</div>
</div>
<label>URL</label>
<input id="burl" placeholder="http://ejemplo.com/accion">
<div class="r2">
<div><label>Metodo</label>
<select id="bmethod" onchange="togPL()">
<option value="0">GET</option>
<option value="1">POST</option>
</select></div>
<div><label>Timeout (seg)</label>
<input id="btimeout" type="number" min="1" max="30" value="5">
</div></div>
<label>Cooldown (seg)</label>
<input id="bcooldown" type="number" step="0.5" min="0.5" max="60" value="2">
<div id="pl-row" class="pl-row">
<label>Payload (JSON)</label>
<input id="bpayload" placeholder='{"key":"value"}'>
</div>
<button class="btn btn-ok" onclick="saveBtn()">Guardar Boton</button>
<button class="btn btn-test" onclick="testBtn()">Probar Boton</button>
<div id="bmsg" class="msg"></div>
</div>
</div>

<!-- LED -->
<div id="s-led" class="sec">
<div class="card">
<h2>💡 LED RGB</h2>
<div class="lprev" id="lprev"></div>
<div class="cgrid">
<div class="cbtn" style="background:#e00" onclick="sLed(40,0,0,this)"></div>
<div class="cbtn" style="background:#0c0" onclick="sLed(0,40,0,this)"></div>
<div class="cbtn" style="background:#00e" onclick="sLed(0,0,40,this)"></div>
<div class="cbtn" style="background:#ee0" onclick="sLed(40,40,0,this)"></div>
<div class="cbtn" style="background:#e0e" onclick="sLed(40,0,40,this)"></div>
<div class="cbtn" style="background:#0ee" onclick="sLed(0,40,40,this)"></div>
<div class="cbtn" style="background:#eee" onclick="sLed(40,40,40,this)"></div>
<div class="cbtn" style="background:#e80" onclick="sLed(40,20,0,this)"></div>
</div>
<div class="srow">
<label>R <span id="rv">0</span></label>
<input type="range" id="sr" min="0" max="255" value="0" oninput="updSl()">
</div>
<div class="srow">
<label>G <span id="gv">0</span></label>
<input type="range" id="sg" min="0" max="255" value="0" oninput="updSl()">
</div>
<div class="srow">
<label>B <span id="bv">0</span></label>
<input type="range" id="sb" min="0" max="255" value="0" oninput="updSl()">
</div>
<button class="btn btn-ok" onclick="applySl()">Aplicar Color</button>
<button class="btn btn-off" onclick="ledOff()">Apagar LED</button>
<div id="ledmsg" class="msg"></div>
</div>
</div>

<!-- Sistema -->
<div id="s-sys" class="sec">
<div class="card">
<h2>📡 Red</h2>
<button class="btn btn-p" onclick="showNet()">Ver Info de Red</button>
</div>
<div class="card">
<h2>📦 Firmware OTA</h2>
<input type="file" id="otafile" accept=".bin">
<button class="btn btn-ok" onclick="doOta()">Subir Firmware</button>
<div id="omsg" class="msg"></div>
</div>
<div class="card">
<h2>🔑 Credenciales y Ajustes</h2>
<label>Usuario</label>
<input id="auser" placeholder="admin">
<label>Nueva contraseña</label>
<div class="pw">
<input id="apass" type="password" placeholder="Nueva contraseña">
<button class="pw-tog" type="button" onclick="togVis('apass')">🗝️</button>
</div>
<label>Confirmar contraseña</label>
<div class="pw">
<input id="apass2" type="password" placeholder="Repetir contraseña">
<button class="pw-tog" type="button" onclick="togVis('apass2')">🗝️</button>
</div>
<label>Tiempo Factory Reset (seg)</label>
<input id="areset" type="number" min="3" max="60" value="8">
<button class="btn btn-danger" onclick="saveAdmin()">Guardar Ajustes</button>
<div id="amsg" class="msg"></div>
</div>
</div>

<nav class="nav">
<div class="nav-item act" onclick="go('wifi',this)"><span class="ico">📶</span>WiFi</div>
<div class="nav-item" onclick="go('btn',this)"><span class="ico">🔘</span>Botones</div>
<div class="nav-item" onclick="go('led',this)"><span class="ico">💡</span>LED</div>
<div class="nav-item" onclick="go('sys',this)"><span class="ico">⚙️</span>Sistema</div>
</nav>
</div>

<div id="netmodal" class="modal-bg" onclick="if(event.target===this)closeNet()">
<div class="modal">
<h2>📡 Info de Red</h2>
<div id="netbody"><div style="text-align:center;color:#aaa;padding:20px">Cargando...</div></div>
<button class="btn btn-p" onclick="closeNet()">Cerrar</button>
</div>
</div>

<script>
var A='',cBtn=1,curP='';
function go(s,el){
document.querySelectorAll('.sec').forEach(function(x){x.classList.remove('act')});
document.querySelectorAll('.nav-item').forEach(function(x){x.classList.remove('act')});
document.getElementById('s-'+s).classList.add('act');el.classList.add('act');
}
function doLogin(){
var u=$('luser').value,p=$('lpass').value;
if(!u||!p){msg('lmsg',0,'Introduce usuario y contraseña');return;}
A='Basic '+btoa(u+':'+p);
fetch('/api/verify',{headers:{'Authorization':A}}).then(function(r){
if(r.ok){$('login').classList.add('hid');$('app').classList.remove('hid');$('auser').value=u;curP=p;loadBtn(1);loadSys();}
else{A='';msg('lmsg',0,'Usuario o contraseña incorrectos');}
}).catch(function(){msg('lmsg',0,'Error de conexion')});
}
function af(u,o){o=o||{};o.headers=o.headers||{};o.headers['Authorization']=A;return fetch(u,o);}
function $(id){return document.getElementById(id);}
function swTab(n,el){cBtn=n;document.querySelectorAll('.tab').forEach(function(t){t.classList.remove('act')});el.classList.add('act');loadBtn(n);}
function togVis(id){var p=$(id);p.type=p.type==='password'?'text':'password';}
function togPL(){$('pl-row').classList.toggle('show',$('bmethod').value==='1');}
function msg(id,ok,t){var m=$(id);m.className='msg '+(ok?'ok':'err');m.textContent=t;setTimeout(function(){m.className='msg';},4000);}
function scanWifi(){
var l=$('wlist');l.innerHTML='<div style="text-align:center;padding:10px;color:#aaa">Buscando...</div>';
af('/api/scan').then(function(r){return r.json()}).then(function(d){
if(!d.length){l.innerHTML='<div style="text-align:center;padding:10px;color:#aaa">No se encontraron redes</div>';return;}
l.innerHTML='';d.forEach(function(ap){
var e=document.createElement('div');e.className='wifi-item';
var s=ap.rssi>-50?'▂▄▆█':ap.rssi>-70?'▂▄▆_':'▂▄__';
e.innerHTML='<span class="name">'+ap.ssid+'</span><span class="rssi">'+s+' '+ap.rssi+'dBm</span><span class="lock">'+(ap.auth>0?'🔒':'')+'</span>';
e.onclick=function(){document.querySelectorAll('.wifi-item').forEach(function(i){i.classList.remove('sel')});e.classList.add('sel');$('ssid').value=ap.ssid;$('wpass').focus();};
l.appendChild(e);});
}).catch(function(){l.innerHTML='';msg('wmsg',0,'Error al buscar');});
}
function saveWifi(){
var s=$('ssid').value,p=$('wpass').value;
if(!s){msg('wmsg',0,'Introduce un SSID');return;}
af('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,pass:p})
}).then(function(r){return r.json()}).then(function(d){msg('wmsg',d.ok,d.ok?'Guardado. Reiniciando...':'Error');if(d.ok)setTimeout(function(){location.reload()},3000);
}).catch(function(){msg('wmsg',0,'Error de conexion')});
}
function gBD(){return{id:cBtn,url:$('burl').value,method:parseInt($('bmethod').value),payload:$('bpayload').value||'',timeout:parseInt($('btimeout').value||'5')*1000,cooldown:parseFloat($('bcooldown').value||'2')*1000};}
function saveBtn(){
var d=gBD();if(!d.url){msg('bmsg',0,'Introduce una URL');return;}
af('/api/btn',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)
}).then(function(r){return r.json()}).then(function(d){msg('bmsg',d.ok,d.ok?'Boton '+cBtn+' guardado':'Error');
}).catch(function(){msg('bmsg',0,'Error de conexion')});
}
function testBtn(){
var d=gBD();if(!d.url){msg('bmsg',0,'Introduce una URL');return;}
msg('bmsg',1,'Probando...');
af('/api/test',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)
}).then(function(r){return r.json()}).then(function(d){
if(d.status<0)msg('bmsg',0,'Error de conexion (status: '+d.status+')');
else if(d.ok)msg('bmsg',1,'OK - Status: '+d.status);
else msg('bmsg',0,'Error - Status: '+d.status);
}).catch(function(){msg('bmsg',0,'Error de conexion')});
}
function loadBtn(n){
af('/api/btn?id='+n).then(function(r){return r.json()}).then(function(d){
$('burl').value=d.url||'';$('bmethod').value=d.method||0;$('bpayload').value=d.payload||'';
$('btimeout').value=Math.round((d.timeout||5000)/1000);$('bcooldown').value=(d.cooldown||2000)/1000;togPL();
}).catch(function(){$('burl').value='';$('bmethod').value=0;$('bpayload').value='';$('btimeout').value=5;$('bcooldown').value=2;togPL();});
}
function loadSys(){
af('/api/admin').then(function(r){return r.json()}).then(function(d){
if(d.user)$('auser').value=d.user;
if(d.reset_time)$('areset').value=d.reset_time;
}).catch(function(){});
}
function saveAdmin(){
var u=$('auser').value,p=$('apass').value,p2=$('apass2').value,r=$('areset').value;
if(!u){msg('amsg',0,'Introduce un usuario');return;}
if(p && p!==p2){msg('amsg',0,'Las contraseñas no coinciden');return;}
if(p && p.length<4){msg('amsg',0,'Minimo 4 caracteres');return;}
var body={user:u,reset_time:parseInt(r)};
if(p)body.pass=p;
af('/api/admin',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)
}).then(function(res){return res.json()}).then(function(d){
if(d.ok){curP=p?p:curP;A='Basic '+btoa(u+':'+curP);msg('amsg',1,'Ajustes guardados');$('apass').value='';$('apass2').value='';}
else msg('amsg',0,'Error al guardar');
}).catch(function(){msg('amsg',0,'Error de conexion')});
}
function doOta(){
var f=$('otafile');if(!f.files||!f.files[0]){msg('omsg',0,'Selecciona un archivo .bin');return;}
msg('omsg',1,'Subiendo firmware... NO APAGUES');
af('/api/ota',{method:'POST',body:f.files[0]}).then(function(r){return r.json()}).then(function(d){
if(d.ok){msg('omsg',1,'Actualizado. Reiniciando...');setTimeout(function(){location.reload()},10000);}
else msg('omsg',0,'Error al actualizar');
}).catch(function(){msg('omsg',0,'Error de conexion');});
}
function sLed(r,g,b,el){
document.querySelectorAll('.cbtn').forEach(function(c){c.classList.remove('act')});
if(el)el.classList.add('act');
$('sr').value=r;$('sg').value=g;$('sb').value=b;
$('rv').textContent=r;$('gv').textContent=g;$('bv').textContent=b;
$('lprev').style.backgroundColor='rgb('+r+','+g+','+b+')';
af('/api/led',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({r:r,g:g,b:b})
}).catch(function(){msg('ledmsg',0,'Error');});
}
function updSl(){
var r=+$('sr').value,g=+$('sg').value,b=+$('sb').value;
$('rv').textContent=r;$('gv').textContent=g;$('bv').textContent=b;
$('lprev').style.backgroundColor='rgb('+r+','+g+','+b+')';
document.querySelectorAll('.cbtn').forEach(function(c){c.classList.remove('act')});
}
function applySl(){sLed(+$('sr').value,+$('sg').value,+$('sb').value,null);}
function ledOff(){
document.querySelectorAll('.cbtn').forEach(function(c){c.classList.remove('act')});
$('lprev').style.backgroundColor='transparent';
$('sr').value=0;$('sg').value=0;$('sb').value=0;
$('rv').textContent='0';$('gv').textContent='0';$('bv').textContent='0';
af('/api/led',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({off:true})
}).catch(function(){msg('ledmsg',0,'Error');});
}
function showNet(){
$('netmodal').classList.add('show');
$('netbody').innerHTML='<div style="text-align:center;color:#aaa;padding:20px">Cargando...</div>';
af('/api/netinfo').then(function(r){return r.json()}).then(function(d){
var h='',rows=[['SSID',d.ssid||'No conectado'],['RSSI',d.rssi?d.rssi+' dBm':'\u2014'],['IP',d.ip||'0.0.0.0'],['Mascara',d.mask||'\u2014'],['Gateway',d.gw||'\u2014'],['DNS',d.dns||'\u2014'],['MAC',d.mac||'\u2014'],['IP AP',d.ap_ip||'\u2014']];
rows.forEach(function(r){h+='<div class="irow"><span class="il">'+r[0]+'</span><span class="iv">'+r[1]+'</span></div>';});
$('netbody').innerHTML=h;
}).catch(function(){$('netbody').innerHTML='<div style="text-align:center;color:#f87171;padding:20px">Error</div>';});
}
function closeNet(){$('netmodal').classList.remove('show');}
$('lpass').addEventListener('keyup',function(e){if(e.key==='Enter')doLogin();});
</script>
</body></html>
)rawliteral";
