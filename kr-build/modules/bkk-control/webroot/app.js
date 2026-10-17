const control = '/data/adb/modules/bkk-control/bkctl';
let counter = 0;
let pending = false;
const byId = (id) => document.getElementById(id);

function exec(args) {
  return new Promise((resolve, reject) => {
    const callback = `bkk_${Date.now()}_${counter++}`;
    const timeout = setTimeout(() => {
      delete globalThis[callback];
      reject(new Error('命令超时，请刷新状态'));
    }, 15000);
    globalThis[callback] = (errno, stdout, stderr) => {
      clearTimeout(timeout);
      delete globalThis[callback];
      if (errno !== 0) reject(new Error(stderr || `退出码 ${errno}`));
      else resolve(stdout);
    };
    try {
      ksu.exec(`${control} ${args}`, '{}', callback);
    } catch (error) {
      clearTimeout(timeout);
      delete globalThis[callback];
      reject(error);
    }
  });
}

async function refresh() {
  const text = await exec('status');
  const values = Object.fromEntries(text.trim().split('\n').map((line) => {
    const split = line.indexOf('=');
    return [line.slice(0, split), line.slice(split + 1)];
  }));
  byId('status').textContent = text;
  byId('mode').value = values.configured_mode || 'disabled';
}

async function run(operation) {
  if (pending) return;
  pending = true;
  for (const button of document.querySelectorAll('button')) button.disabled = true;
  byId('mode').disabled = true;
  try {
    await operation();
    byId('message').textContent = '';
  } catch (error) {
    byId('message').textContent = error.message;
  } finally {
    pending = false;
    for (const button of document.querySelectorAll('button')) button.disabled = false;
    byId('mode').disabled = false;
  }
}

byId('refresh').onclick = () => run(refresh);
byId('read-log').onclick = () => run(async () => { byId('log').textContent = await exec('log'); });
byId('restore').onclick = () => run(async () => { await exec('restore'); await refresh(); });
byId('apply').onclick = () => run(async () => {
  const mode = byId('mode').value;
  if (!['disabled', 'auto', 'manual'].includes(mode)) throw new Error('模式无效');
  await exec(`set mode ${mode}`);
  await refresh();
});
run(async () => {
  const response = await fetch('version.json');
  if (!response.ok) throw new Error('模块版本文件缺失');
  byId('version').textContent = (await response.json()).version;
  await refresh();
});
