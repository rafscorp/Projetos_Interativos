import { forwardRef, useEffect, useImperativeHandle, useRef } from 'react'
import { Terminal as XTerm } from '@xterm/xterm'
import { FitAddon } from '@xterm/addon-fit'
import '@xterm/xterm/css/xterm.css'

// componente fino em cima do xterm.js. Ele NAO sabe nada sobre C, Python ou
// Emscripten - so sabe escrever texto na tela e ler uma linha digitada pelo
// usuario. Quem chama (os hooks useWasmProgram/usePyodideProgram) e que
// decide o que fazer com essa linha.
const Terminal = forwardRef(function Terminal({ onReady }, ref) {
  const containerRef = useRef(null)
  const xtermRef = useRef(null)
  const fitAddonRef = useRef(null)
  const lineBufferRef = useRef('')
  const pendingResolveRef = useRef(null)

  useEffect(() => {
    const term = new XTerm({
      convertEol: true,
      fontFamily: "'JetBrains Mono', monospace",
      fontSize: 14,
      theme: {
        background: '#0a0e0f',
        foreground: '#e7f1ee',
        cursor: '#35e3a3',
      },
      cursorBlink: true,
    })
    const fitAddon = new FitAddon()
    term.loadAddon(fitAddon)
    term.open(containerRef.current)
    fitAddon.fit()

    xtermRef.current = term
    fitAddonRef.current = fitAddon

    // cada tecla digitada chega aqui crua (onData). Monto a linha manualmente:
    // Enter resolve a Promise pendente (se tiver alguem esperando input),
    // Backspace apaga o ultimo caractere, o resto so acumula e ecoa na tela.
    const disposable = term.onData((data) => {
      const code = data.charCodeAt(0)
      if (code === 13) {
        // Enter
        const linha = lineBufferRef.current
        lineBufferRef.current = ''
        term.write('\r\n')
        if (pendingResolveRef.current) {
          const resolve = pendingResolveRef.current
          pendingResolveRef.current = null
          resolve(linha)
        }
      } else if (code === 127 || code === 8) {
        // Backspace/Delete
        if (lineBufferRef.current.length > 0) {
          lineBufferRef.current = lineBufferRef.current.slice(0, -1)
          term.write('\b \b')
        }
      } else if (code >= 32) {
        // caractere "normal" (ignora outras teclas de controle tipo setas)
        lineBufferRef.current += data
        term.write(data)
      }
    })

    const resizeObserver = new ResizeObserver(() => fitAddon.fit())
    resizeObserver.observe(containerRef.current)

    if (onReady) onReady()

    return () => {
      disposable.dispose()
      resizeObserver.disconnect()
      term.dispose()
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  useImperativeHandle(ref, () => ({
    write(text) {
      xtermRef.current?.write(text)
    },
    clear() {
      xtermRef.current?.clear()
    },
    reset() {
      xtermRef.current?.reset()
    },
    // devolve uma Promise que so resolve quando o usuario aperta Enter
    readLine() {
      return new Promise((resolve) => {
        pendingResolveRef.current = resolve
      })
    },
    focus() {
      xtermRef.current?.focus()
    },
  }))

  return <div ref={containerRef} className="terminal-container" />
})

export default Terminal
