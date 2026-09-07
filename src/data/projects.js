// fonte de dados central: cada projeto sabe seu tipo (roda de verdade ou so
// mostra codigo), a linguagem, e o que precisa pra rodar (arquivo wasm,
// modulo Python) ou pra so exibir (arquivos de codigo-fonte).

import caixaSrc from '../sources/c/caixa.c?raw'
import calculadoraSrc from '../sources/c/calculadora.c?raw'
import mercadoSrc from '../sources/c/mercado.c?raw'

import gerenciadorSrc from '../sources/python/gerenciador.py?raw'
import bibliotecaMainSrc from '../sources/python/biblioteca_main.py?raw'
import bibliotecaRegrasSrc from '../sources/python/biblioteca_regras.py?raw'
import bibliotecaModelosSrc from '../sources/python/biblioteca_modelos.py?raw'

import calculadoraSwingSrc from '../sources/java/Calculadora_Swing/Calculadora.java?raw'
import produtoSrc from '../sources/java/Sistema_Estoque/Produto.java?raw'
import movimentacaoSrc from '../sources/java/Sistema_Estoque/Movimentacao.java?raw'
import tipoMovimentacaoSrc from '../sources/java/Sistema_Estoque/TipoMovimentacao.java?raw'
import persistivelSrc from '../sources/java/Sistema_Estoque/Persistivel.java?raw'
import relatorioSrc from '../sources/java/Sistema_Estoque/Relatorio.java?raw'
import relatorioBaixoSrc from '../sources/java/Sistema_Estoque/RelatorioEstoqueBaixo.java?raw'
import relatorioMovSrc from '../sources/java/Sistema_Estoque/RelatorioMaisMovimentados.java?raw'
import estoqueExceptionSrc from '../sources/java/Sistema_Estoque/EstoqueException.java?raw'
import gerenciadorEstoqueSrc from '../sources/java/Sistema_Estoque/GerenciadorEstoque.java?raw'
import mainEstoqueSrc from '../sources/java/Sistema_Estoque/Main.java?raw'

// os arquivos .py "web" (com async web_input) sao carregados como texto puro
// (nao ?raw porque nao viram import de UI, viram arquivo escrito no FS virtual
// do Pyodide em tempo de execucao) - uso import.meta.glob com eager pra pegar
// o conteudo de todos de uma vez
const pythonWebFiles = import.meta.glob('../../python-src/**/*.py', { eager: true, query: '?raw', import: 'default' })

function pyFile(path) {
  const key = Object.keys(pythonWebFiles).find((k) => k.endsWith(path))
  if (!key) throw new Error(`Arquivo python-src nao encontrado: ${path}`)
  return pythonWebFiles[key]
}

export const projects = [
  {
    id: 'caixa',
    nome: 'Caixa Eletrônico (CYPRUS)',
    linguagem: 'C',
    nivel: 'Iniciante',
    descricao: 'Simulador de caixa eletrônico com distribuição de cédulas.',
    tipo: 'wasm',
    scriptUrl: '/wasm/caixa.js',
    moduleName: 'CaixaModule',
    arquivos: [{ filename: 'Caixa_Cyprus.c', code: caixaSrc }],
    codeLanguage: 'c',
  },
  {
    id: 'calculadora',
    nome: 'Calculadora Dinâmica',
    linguagem: 'C',
    nivel: 'Intermediário',
    descricao: 'Calculadora científica + uni-funcional, com Fibonacci, PA e sistema de logs.',
    tipo: 'wasm',
    scriptUrl: '/wasm/calculadora.js',
    moduleName: 'CalculadoraModule',
    arquivos: [{ filename: 'Calculadora_Dinamica.c', code: calculadoraSrc }],
    codeLanguage: 'c',
  },
  {
    id: 'mercado',
    nome: 'Mercado - Cadastro',
    linguagem: 'C',
    nivel: 'Avançado',
    descricao: 'Cadastro de produtos e clientes com structs e memória dinâmica.',
    tipo: 'wasm',
    scriptUrl: '/wasm/mercado.js',
    moduleName: 'MercadoModule',
    arquivos: [{ filename: 'main.c', code: mercadoSrc }],
    codeLanguage: 'c',
  },
  {
    id: 'tarefas',
    nome: 'Gerenciador de Tarefas',
    linguagem: 'Python',
    nivel: 'Iniciante',
    descricao: 'To-do list de linha de comando com persistência em JSON.',
    tipo: 'pyodide',
    entryModule: 'gerenciador_web',
    sourceFiles: [{ path: 'gerenciador_web.py', content: pyFile('python-src/gerenciador_web.py') }],
    arquivos: [{ filename: 'gerenciador.py', code: gerenciadorSrc }],
    codeLanguage: 'python',
  },
  {
    id: 'biblioteca',
    nome: 'Sistema de Biblioteca',
    linguagem: 'Python',
    nivel: 'Avançado',
    descricao: 'Empréstimos de livros orientado a objetos, com regras de negócio.',
    tipo: 'pyodide',
    entryModule: 'main_web',
    sourceFiles: [
      { path: 'modelos.py', content: pyFile('biblioteca_web/modelos.py') },
      { path: 'biblioteca.py', content: pyFile('biblioteca_web/biblioteca.py') },
      { path: 'main_web.py', content: pyFile('biblioteca_web/main_web.py') },
    ],
    arquivos: [
      { filename: 'main.py', code: bibliotecaMainSrc },
      { filename: 'biblioteca.py', code: bibliotecaRegrasSrc },
      { filename: 'modelos.py', code: bibliotecaModelosSrc },
    ],
    codeLanguage: 'python',
  },
  {
    id: 'calculadora-swing',
    nome: 'Calculadora (Swing)',
    linguagem: 'Java',
    nivel: 'Intermediário',
    descricao: 'Calculadora com interface gráfica em Java Swing.',
    tipo: 'codigo',
    motivoSemExecucao: 'Java não tem como rodar de verdade no navegador sem um serviço pago de terceiros — aqui dá pra ler o código.',
    arquivos: [{ filename: 'Calculadora.java', code: calculadoraSwingSrc }],
    codeLanguage: 'java',
  },
  {
    id: 'sistema-estoque',
    nome: 'Sistema de Estoque',
    linguagem: 'Java',
    nivel: 'Avançado',
    descricao: 'Controle de estoque orientado a objetos: interfaces, classe abstrata e exceção customizada.',
    tipo: 'codigo',
    motivoSemExecucao: 'Java não tem como rodar de verdade no navegador sem um serviço pago de terceiros — aqui dá pra ler o código.',
    arquivos: [
      { filename: 'Main.java', code: mainEstoqueSrc },
      { filename: 'GerenciadorEstoque.java', code: gerenciadorEstoqueSrc },
      { filename: 'Produto.java', code: produtoSrc },
      { filename: 'Movimentacao.java', code: movimentacaoSrc },
      { filename: 'TipoMovimentacao.java', code: tipoMovimentacaoSrc },
      { filename: 'Persistivel.java', code: persistivelSrc },
      { filename: 'Relatorio.java', code: relatorioSrc },
      { filename: 'RelatorioEstoqueBaixo.java', code: relatorioBaixoSrc },
      { filename: 'RelatorioMaisMovimentados.java', code: relatorioMovSrc },
      { filename: 'EstoqueException.java', code: estoqueExceptionSrc },
    ],
    codeLanguage: 'java',
  },
]
