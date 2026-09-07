import { useCallback, useRef, useState } from 'react'

const CHEERPJ_LOADER = 'https://cjrtnc.leaningtech.com/4.3/loader.js'

let cheerpjReadyPromise = null

// CheerpJ (leaningtech.com) roda bytecode Java de verdade no navegador via
// WebAssembly - gratuito pra uso pessoal, so pedindo credito visivel (por
// isso o link "rodando com CheerpJ" aparece no rodape do card). cheerpjInit()
// so pode ser chamado UMA vez por pagina, entao guardo a Promise numa
// variavel de modulo (fora do componente) pra reaproveitar entre trocas de
// projeto.
function ensureCheerpjReady() {
  if (cheerpjReadyPromise) return cheerpjReadyPromise

  cheerpjReadyPromise = new Promise((resolve, reject) => {
    const script = document.createElement('script')
    script.src = CHEERPJ_LOADER
    script.onload = () => {
      window.cheerpjInit().then(resolve).catch(reject)
    }
    script.onerror = () => reject(new Error('Falha ao carregar o runtime do CheerpJ'))
    document.body.appendChild(script)
  })

  return cheerpjReadyPromise
}

// "/app/" e um mount virtual do CheerpJ que aponta pra raiz do servidor
// HTTP que serviu a pagina - por isso concateno com BASE_URL (que ja tem
// o prefixo "/Projetos_Interativos/" configurado no vite.config.js) pra
// achar o .jar no lugar certo tanto local quanto no GitHub Pages.
//
// programas de CONSOLE (sem Swing/AWT) nao tem uma janela grafica pra
// desenhar - o CheerpJ manda o println deles direto pro console.log do
// devtools, o que ninguem visitando o site vai ver. Pra resolver isso sem
// precisar reescrever o Java com metodos nativos (bem mais trabalho),
// intercepto o console.log ENQUANTO esse programa especifico esta rodando
// e espelho cada linha na tela tambem - o devtools continua recebendo
// normalmente (chamo o console.log original em seguida).
export function useCheerpjProgram({ jarPath, displayRef, isConsoleApp }) {
  const [status, setStatus] = useState('idle') // idle | loading | running | error
  const [errorMsg, setErrorMsg] = useState(null)
  const [consoleLines, setConsoleLines] = useState([])
  const startedRef = useRef(false)

  const run = useCallback(async () => {
    const container = displayRef.current
    if (startedRef.current) return

    setStatus('loading')
    setErrorMsg(null)
    setConsoleLines([])

    try {
      await ensureCheerpjReady()

      if (isConsoleApp) {
        // apps de console (sem Swing/AWT) nao tem uma janela grafica - o
        // CheerpJ manda os println() deles pro console.log do devtools por
        // padrao. Intercepto aqui pra espelhar cada linha na tela tambem.
        // (a ENTRADA de teclado pro Scanner desses apps, por outro lado, eu
        // nao consegui rotear de fora - fica so leitura por enquanto)
        const originalLog = console.log.bind(console)
        console.log = (...args) => {
          originalLog(...args)
          setConsoleLines((prev) => [...prev, args.map(String).join(' ')])
        }
      } else if (container) {
        container.innerHTML = ''
        window.cheerpjCreateDisplay(-1, -1, container)
      }

      setStatus('running')
      startedRef.current = true
      const jarUrl = `/app${import.meta.env.BASE_URL}${jarPath}`
      await window.cheerpjRunJar(jarUrl)
    } catch (err) {
      console.error(err)
      setStatus('error')
      setErrorMsg(err.message || String(err))
    }
  }, [jarPath, displayRef, isConsoleApp])

  return { run, status, errorMsg, consoleLines }
}
