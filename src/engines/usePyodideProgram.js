import { useCallback, useRef, useState } from 'react'

const PYODIDE_CDN = 'https://cdn.jsdelivr.net/pyodide/v0.26.4/full/pyodide.js'

let pyodidePromise = null

// carrega o Pyodide (Python compilado pra WebAssembly) uma unica vez e
// reaproveita entre os projetos, pra nao ter que baixar de novo a cada troca
function loadPyodideOnce() {
  if (pyodidePromise) return pyodidePromise

  pyodidePromise = new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[data-pyodide]`)
    if (existing) {
      existing.addEventListener('load', () => resolve(window.loadPyodide()))
      return
    }
    const script = document.createElement('script')
    script.src = PYODIDE_CDN
    script.dataset.pyodide = 'true'
    script.onload = () => resolve(window.loadPyodide())
    script.onerror = () => reject(new Error('Falha ao carregar o Pyodide via CDN'))
    document.body.appendChild(script)
  })

  return pyodidePromise
}

// hook generico pra rodar QUALQUER script Python adaptado com o padrao
// `async def ... await web_input(...)` documentado em python-src/. Recebe
// o codigo fonte (string) e o nome da funcao async de entrada (normalmente
// "main"), e cuida de: carregar o Pyodide, ligar print()/stdin na Terminal,
// rodar o script e por fim chamar `await main()`.
export function usePyodideProgram({ sourceFiles, entryModule, terminalRef }) {
  const [status, setStatus] = useState('idle') // idle | loading | running | finished | error
  const [errorMsg, setErrorMsg] = useState(null)
  const pyodideRef = useRef(null)

  const run = useCallback(async () => {
    const term = terminalRef.current
    if (!term) return

    setStatus('loading')
    setErrorMsg(null)
    term.reset()

    globalThis.__webReadLine = () => term.readLine()

    try {
      if (!pyodideRef.current) {
        term.write('carregando Python (Pyodide)... isso so demora na primeira vez\r\n')
        pyodideRef.current = await loadPyodideOnce()
      }
      const pyodide = pyodideRef.current

      pyodide.setStdout({
        batched: (text) => term.write(text.replace(/\n/g, '\r\n') + '\r\n'),
      })
      pyodide.setStderr({
        batched: (text) => term.write(`\x1b[31m${text.replace(/\n/g, '\r\n')}\x1b[0m\r\n`),
      })

      setStatus('running')

      // registra cada arquivo fonte como um "modulo" dentro do sistema de
      // arquivos virtual do Pyodide, pra que os `from biblioteca import X`
      // dentro do proprio codigo Python funcionem igual funcionariam com
      // arquivos de verdade numa pasta
      for (const file of sourceFiles) {
        pyodide.FS.writeFile(file.path, file.content)
      }

      pyodide.runPython(`import sys; sys.path.insert(0, '.')`)
      await pyodide.runPythonAsync(`
import importlib
mod = importlib.import_module("${entryModule}")
importlib.reload(mod)
await mod.main()
`)

      setStatus('finished')
      term.write('\r\n\x1b[2m[programa encerrado - clique em "Rodar de novo" pra reiniciar]\x1b[0m\r\n')
    } catch (err) {
      console.error(err)
      setStatus('error')
      setErrorMsg(err.message || String(err))
      term.write(`\r\n\x1b[31mErro ao rodar o programa: ${err.message || err}\x1b[0m\r\n`)
    }
  }, [sourceFiles, entryModule, terminalRef])

  return { run, status, errorMsg }
}
