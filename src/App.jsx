import { useMemo, useRef, useState } from 'react'
import Terminal from './components/Terminal'
import CodeViewer from './components/CodeViewer'
import { useWasmProgram } from './engines/useWasmProgram'
import { usePyodideProgram } from './engines/usePyodideProgram'
import { useCheerpjProgram } from './engines/useCheerpjProgram'
import { projects } from './data/projects'
import './App.css'

const LINGUAGENS = ['C', 'Python', 'Java']

function RunButtonLabel({ status }) {
  if (status === 'loading') return 'Carregando...'
  if (status === 'running') return 'Rodando...'
  if (status === 'idle') return 'Rodar'
  return 'Rodar de novo'
}

function ProjectRunner({ project }) {
  const terminalRef = useRef(null)
  const displayRef = useRef(null)
  const [view, setView] = useState('run') // 'run' | 'code'

  const wasmProgram = useWasmProgram({
    scriptUrl: project.scriptUrl,
    moduleName: project.moduleName,
    terminalRef,
  })
  const pyodideProgram = usePyodideProgram({
    sourceFiles: project.sourceFiles,
    entryModule: project.entryModule,
    terminalRef,
  })
  const cheerpjProgram = useCheerpjProgram({
    jarPath: project.jarPath,
    displayRef,
    isConsoleApp: project.isConsoleApp,
  })

  const engine =
    project.tipo === 'wasm' ? wasmProgram : project.tipo === 'pyodide' ? pyodideProgram : project.tipo === 'cheerpj' ? cheerpjProgram : null

  const usaTerminal = project.tipo === 'wasm' || project.tipo === 'pyodide'
  const usaCheerpj = project.tipo === 'cheerpj'

  return (
    <div className="project-runner">
      <div className="project-header">
        <div>
          <h2>{project.nome}</h2>
          <p className="project-desc">{project.descricao}</p>
        </div>
        <div className="project-badges">
          <span className={`badge lang-${project.linguagem.toLowerCase()}`}>{project.linguagem}</span>
          <span className="badge">{project.nivel}</span>
        </div>
      </div>

      {engine && (
        <div className="view-tabs">
          <button className={view === 'run' ? 'active' : ''} onClick={() => setView('run')}>
            ▶ Rodar
          </button>
          <button className={view === 'code' ? 'active' : ''} onClick={() => setView('code')}>
            {'</>'} Código-fonte
          </button>
        </div>
      )}

      {engine && view === 'run' && (
        <div className="run-panel">
          <div className="run-toolbar">
            <button className="btn-run" onClick={engine.run} disabled={engine.status === 'loading' || engine.status === 'running'}>
              <RunButtonLabel status={engine.status} />
            </button>
            <span className="hint">{project.ferramenta}</span>
          </div>

          {usaTerminal && <Terminal ref={terminalRef} />}

          {usaCheerpj && !project.isConsoleApp && <div ref={displayRef} className="cheerpj-display" />}

          {usaCheerpj && project.isConsoleApp && (
            <div className="cheerpj-console">
              <pre className="cheerpj-console-log">
                {cheerpjProgram.consoleLines.join('\n') || (engine.status === 'running' ? 'carregando...' : '')}
              </pre>
              {engine.status === 'running' && (
                <p className="cheerpj-console-note">
                  ⚠️ input do Scanner não tá funcionando aqui ainda — o resto roda de verdade
                </p>
              )}
            </div>
          )}

          {usaCheerpj && (
            <p className="cheerpj-credit">
              rodando com <a href="https://cheerpj.com" target="_blank" rel="noopener">CheerpJ</a>
            </p>
          )}
        </div>
      )}

      {(view === 'code' || !engine) && (
        <CodeViewer files={project.arquivos} language={project.codeLanguage} />
      )}
    </div>
  )
}

export default function App() {
  const [activeId, setActiveId] = useState(projects[0].id)
  const activeProject = useMemo(() => projects.find((p) => p.id === activeId), [activeId])

  return (
    <div className="app-shell">
      <header className="topbar">
        <a href="https://rafscorp.github.io" className="logo">
          rafs<span className="accent">corp</span> <span className="dim">/ projetos interativos</span>
        </a>
        <div className="topbar-links">
          <a href="https://github.com/rafscorp" target="_blank" rel="noopener" className="topbar-link">
            GitHub ↗
          </a>
          <a href="https://www.linkedin.com/in/adrian-rafael-28455a361" target="_blank" rel="noopener" className="topbar-link">
            LinkedIn ↗
          </a>
        </div>
      </header>

      <div className="app-body">
        <nav className="sidebar">
          {LINGUAGENS.map((lang) => (
            <div key={lang} className="sidebar-group">
              <p className="sidebar-group-label">{lang}</p>
              {projects
                .filter((p) => p.linguagem === lang)
                .map((p) => (
                  <button
                    key={p.id}
                    className={`sidebar-item ${p.id === activeId ? 'active' : ''}`}
                    onClick={() => setActiveId(p.id)}
                  >
                    {p.nome}
                  </button>
                ))}
            </div>
          ))}
        </nav>

        <main className="main-panel">
          <ProjectRunner key={activeProject.id} project={activeProject} />
        </main>
      </div>
    </div>
  )
}
