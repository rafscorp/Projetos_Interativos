import { useCallback, useRef, useState } from 'react'

// hook generico pra rodar QUALQUER programa em C compilado com o padrao
// EM_ASYNC_JS(web_read_line)/EM_JS(web_clear_screen) que documentei no
// wasm-src/. Ele carrega o arquivo .js gerado pelo emcc (modo MODULARIZE)
// dinamicamente via <script>, porque MODULARIZE exporta uma funcao global
// (ex: window.CaixaModule) em vez de um modulo ES importavel direto.
export function useWasmProgram({ scriptUrl, moduleName, terminalRef }) {
  const [status, setStatus] = useState('idle') // idle | loading | running | finished | error
  const [errorMsg, setErrorMsg] = useState(null)
  const scriptLoadedRef = useRef(false)

  const loadScriptOnce = useCallback(async () => {
    if (scriptLoadedRef.current) return
    await new Promise((resolve, reject) => {
      const existing = document.querySelector(`script[data-wasm-src="${scriptUrl}"]`)
      if (existing) {
        resolve()
        return
      }
      const script = document.createElement('script')
      script.src = scriptUrl
      script.dataset.wasmSrc = scriptUrl
      script.onload = resolve
      script.onerror = () => reject(new Error(`Falha ao carregar ${scriptUrl}`))
      document.body.appendChild(script)
    })
    scriptLoadedRef.current = true
  }, [scriptUrl])

  const run = useCallback(async () => {
    const term = terminalRef.current
    if (!term) return

    setStatus('loading')
    setErrorMsg(null)
    term.reset()

    // essas duas globais sao a "ponte" que o codigo C (via EM_ASYNC_JS/EM_JS)
    // chama por baixo dos panos - ver wasm-src/*.c
    globalThis.__webReadLine = () => term.readLine()
    globalThis.__webClearScreen = () => term.clear()

    try {
      await loadScriptOnce()
      const factory = window[moduleName]
      if (typeof factory !== 'function') {
        throw new Error(`Modulo global "${moduleName}" nao foi encontrado apos carregar o script.`)
      }

      setStatus('running')
      await factory({
        print: (text) => term.write(text + '\r\n'),
        printErr: (text) => term.write(`\x1b[31m[erro] ${text}\x1b[0m\r\n`),
      })
      setStatus('finished')
      term.write('\r\n\x1b[2m[programa encerrado - clique em "Rodar de novo" pra reiniciar]\x1b[0m\r\n')
    } catch (err) {
      console.error(err)
      setStatus('error')
      setErrorMsg(err.message || String(err))
      term.write(`\r\n\x1b[31mErro ao rodar o programa: ${err.message || err}\x1b[0m\r\n`)
    }
  }, [loadScriptOnce, moduleName, terminalRef])

  return { run, status, errorMsg }
}
