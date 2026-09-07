# 🧪 Projetos Interativos — Rafael Costa

![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?style=flat-square&logo=webassembly&logoColor=white)
![Emscripten](https://img.shields.io/badge/Emscripten-C%2FC++_→_WASM-e8a33d?style=flat-square)
![Pyodide](https://img.shields.io/badge/Pyodide-Python_no_navegador-3776AB?style=flat-square&logo=python&logoColor=white)
![CheerpJ](https://img.shields.io/badge/CheerpJ-Java_no_navegador-ED8B00?style=flat-square&logo=openjdk&logoColor=white)
[![Rodar ao vivo](https://img.shields.io/badge/▶_rodar_ao_vivo-no_navegador-35e3a3?style=flat-square)](https://rafscorp.github.io/Projetos_Interativos/)

Os projetos dos outros repositórios, rodando de verdade no navegador. Sem instalar nada.

🔗 **[rafscorp.github.io/Projetos_Interativos](https://rafscorp.github.io/Projetos_Interativos/)**

---

## O que roda aqui

- **C** → compilado pra WebAssembly com Emscripten. Terminal de verdade (xterm.js).
- **Python** → Pyodide (CPython em WASM).
- **Java** → CheerpJ (JVM em WASM). A calculadora (Swing) roda 100%. O sistema de estoque roda também, mas o input do Scanner ainda não conectei — só a saída aparece.

## Por baixo dos panos

O C original usa `scanf`/`system("cls")`/`sleep()`, que não existem num navegador. Troquei isso por versões que conversam com o JS: `web_read_line()` pausa o WASM até o usuário digitar (via ASYNCIFY), `web_clear_screen()` limpa o terminal, `emscripten_sleep()` no lugar do sleep. Código adaptado em `wasm-src/`.

No Python, `input()` não funciona direto no Pyodide, então virou `async def` + `await web_input(...)`. Código em `python-src/`.

## Rodando local

```bash
npm install
npm run dev
```

Recompilar o C (precisa do Emscripten instalado):

```bash
emcc wasm-src/caixa_web.c -O2 -s ASYNCIFY -s EXIT_RUNTIME=1 -s MODULARIZE=1 -s EXPORT_NAME=CaixaModule -s ENVIRONMENT=web -o public/wasm/caixa.js
```

## Projetos originais

- [Projetos_em_C](https://github.com/rafscorp/Projetos_em_C)
- [Projetos_em_Python](https://github.com/rafscorp/Projetos_em_Python)
- [Projetos_em_Java](https://github.com/rafscorp/Projetos_em_Java)

## Autor

Rafael Costa — [github.com/rafscorp](https://github.com/rafscorp)
