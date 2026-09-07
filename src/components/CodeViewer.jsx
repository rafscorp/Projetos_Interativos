import { useState } from 'react'
import { PrismLight as SyntaxHighlighter } from 'react-syntax-highlighter'
import c from 'react-syntax-highlighter/dist/esm/languages/prism/c'
import python from 'react-syntax-highlighter/dist/esm/languages/prism/python'
import java from 'react-syntax-highlighter/dist/esm/languages/prism/java'
import { oneDark } from 'react-syntax-highlighter/dist/esm/styles/prism'

// registra só as 3 linguagens que a gente realmente usa - o build "Prism"
// completo carrega mais de 300 linguagens de uma vez, deixando o app lento
// a toa
SyntaxHighlighter.registerLanguage('c', c)
SyntaxHighlighter.registerLanguage('python', python)
SyntaxHighlighter.registerLanguage('java', java)

// visualizador de codigo com syntax highlight - usado tanto pra mostrar o
// codigo-fonte dos projetos que RODAM (C/Python, como complemento) quanto
// pros projetos que so tem visualizacao (Java, que nao roda no navegador)
export default function CodeViewer({ files, language }) {
  const [activeIndex, setActiveIndex] = useState(0)
  const activeFile = files[activeIndex]

  return (
    <div className="code-viewer">
      {files.length > 1 && (
        <div className="code-tabs">
          {files.map((file, i) => (
            <button
              key={file.filename}
              className={`code-tab ${i === activeIndex ? 'active' : ''}`}
              onClick={() => setActiveIndex(i)}
            >
              {file.filename}
            </button>
          ))}
        </div>
      )}
      <div className="code-scroll">
        <SyntaxHighlighter
          language={language}
          style={oneDark}
          showLineNumbers
          customStyle={{ margin: 0, background: 'transparent', fontSize: '13px' }}
        >
          {activeFile.code}
        </SyntaxHighlighter>
      </div>
    </div>
  )
}
