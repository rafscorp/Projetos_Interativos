# 🧪 Projetos Interativos — Rafael Costa

![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?style=flat-square&logo=webassembly&logoColor=white)
![Emscripten](https://img.shields.io/badge/Emscripten-C%2FC++_→_WASM-e8a33d?style=flat-square)
![Pyodide](https://img.shields.io/badge/Pyodide-Python_no_navegador-3776AB?style=flat-square&logo=python&logoColor=white)
![CheerpJ](https://img.shields.io/badge/CheerpJ-Java_no_navegador-ED8B00?style=flat-square&logo=openjdk&logoColor=white)
[![Rodar ao vivo](https://img.shields.io/badge/▶_rodar_ao_vivo-no_navegador-35e3a3?style=flat-square)](https://rafscorp.github.io/Projetos_Interativos/)

Uma central onde dá pra **rodar de verdade** (ou pelo menos ler com carinho) os projetos dos outros repositórios, direto no navegador — sem instalar nada.

🔗 **[Ver ao vivo](https://rafscorp.github.io/Projetos_Interativos/)**

---

## O que tem aqui

| Linguagem | Como funciona |
|---|---|
| **C** | Compilado de verdade pra WebAssembly com [Emscripten](https://emscripten.org/), rodando num terminal (`xterm.js`) dentro da página. `scanf`/`system("cls")`/`sleep()` foram adaptados pra uma versão assíncrona que conversa com o JavaScript (ver `wasm-src/`). |
| **Python** | Roda de verdade com [Pyodide](https://pyodide.org/) (CPython compilado pra WebAssembly). As funções que usam `input()` viraram `async def` + `await` (ver `python-src/`). |
| **Java** | Não tem como rodar JVM no navegador sem depender de um serviço pago de terceiros — aqui fica como visualizador de código com syntax highlight. |

## Como foi feito (resumo técnico)

Pra C: cada `scanf`/`fgets` vira uma chamada a uma função `web_read_line()` (via `EM_ASYNC_JS`) que pausa a execução do WebAssembly (usando `ASYNCIFY`) até o usuário digitar algo no terminal e apertar Enter — sem travar a aba com `prompt()` nativo. `system("cls")` vira uma chamada a `web_clear_screen()`, e `sleep()`/`usleep()` viram `emscripten_sleep()`.

Pra Python: como Pyodide não suporta `input()` assíncrono nativamente, as funções que leem entrada do usuário foram convertidas pra `async def`, trocando `input(...)` por `await web_input(...)` — que por baixo dos panos faz `await` numa Promise do JavaScript.

## Estrutura

```
wasm-src/       # versões .c adaptadas pra WebAssembly (fonte da compilação)
python-src/     # versões .py adaptadas com async/await
src/sources/    # código-fonte ORIGINAL (sem adaptação) só pra exibição
src/engines/    # hooks React que fazem a ponte com Emscripten e Pyodide
src/components/ # Terminal (xterm.js) e visualizador de código
public/wasm/    # arquivos .js/.wasm compilados
docs/           # build de produção (GitHub Pages serve daqui)
```

## Rodando localmente

```bash
npm install
npm run dev
```

Pra recompilar os arquivos C (precisa do [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) instalado e ativado):

```bash
emcc wasm-src/caixa_web.c -O2 -s ASYNCIFY -s EXIT_RUNTIME=1 -s MODULARIZE=1 -s EXPORT_NAME=CaixaModule -s ENVIRONMENT=web -o public/wasm/caixa.js
```

## Projetos originais

- [Projetos_em_C](https://github.com/rafscorp/Projetos_em_C)
- [Projetos_em_Python](https://github.com/rafscorp/Projetos_em_Python)
- [Projetos_em_Java](https://github.com/rafscorp/Projetos_em_Java)

## 👤 Autor

**Rafael Costa**
GitHub: [github.com/rafscorp](https://github.com/rafscorp)
