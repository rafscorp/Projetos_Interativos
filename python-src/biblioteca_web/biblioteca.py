"""
Classe Biblioteca: e o "cerebro" do sistema.

Separei isso do modelos.py de proposito: modelos.py so tem as entidades
(dados + regrinhas isoladas), enquanto aqui fica a logica que precisa
enxergar tudo ao mesmo tempo (todos os livros, todos os membros, todos os
emprestimos) pra decidir se uma operacao pode acontecer ou nao. Separar
assim deixa mais facil de testar cada parte e evita um arquivo gigante
misturando tudo.
"""

import json
import os
from dataclasses import asdict
from datetime import date, timedelta
from typing import Dict, List, Optional

from modelos import Livro, Membro, Emprestimo

PRAZO_EMPRESTIMO_DIAS = 14       # todo emprestimo novo vence 14 dias depois de hoje
LIMITE_PADRAO_EMPRESTIMOS = 3    # quantos livros um membro pode ter emprestado ao mesmo tempo, se nao disser outro numero


class Biblioteca:
    def __init__(self) -> None:
        self.livros: Dict[str, Livro] = {}       # isbn -> Livro
        self.membros: Dict[int, Membro] = {}     # id -> Membro
        self.emprestimos: List[Emprestimo] = []
        self.proximo_id_membro = 1
        self.proximo_id_emprestimo = 1

    # ---------- cadastro ----------

    def cadastrar_livro(self, isbn: str, titulo: str, autor: str, ano: int) -> Livro:
        isbn = isbn.strip()
        if not isbn:
            raise ValueError("ISBN nao pode ficar em branco.")
        if isbn in self.livros:
            raise ValueError(f"Ja existe um livro cadastrado com o ISBN {isbn}.")
        if not titulo.strip():
            raise ValueError("Titulo nao pode ficar em branco.")

        livro = Livro(isbn=isbn, titulo=titulo.strip(), autor=autor.strip(), ano=ano)
        self.livros[isbn] = livro
        return livro

    def cadastrar_membro(self, nome: str, limite_emprestimos: int = LIMITE_PADRAO_EMPRESTIMOS) -> Membro:
        if not nome.strip():
            raise ValueError("Nome nao pode ficar em branco.")
        if limite_emprestimos <= 0:
            raise ValueError("Limite de emprestimos precisa ser maior que zero.")

        membro = Membro(id=self.proximo_id_membro, nome=nome.strip(), limite_emprestimos=limite_emprestimos)
        self.membros[membro.id] = membro
        self.proximo_id_membro += 1
        return membro

    # ---------- consultas auxiliares ----------

    def livro_esta_emprestado(self, isbn: str) -> bool:
        # um livro ta emprestado se existe algum emprestimo dele que ainda nao foi devolvido
        return any(e.isbn_livro == isbn and not e.foi_devolvido() for e in self.emprestimos)

    def emprestimos_ativos_do_membro(self, id_membro: int) -> List[Emprestimo]:
        return [e for e in self.emprestimos if e.id_membro == id_membro and not e.foi_devolvido()]

    def _buscar_emprestimo(self, id_emprestimo: int) -> Optional[Emprestimo]:
        for e in self.emprestimos:
            if e.id == id_emprestimo:
                return e
        return None

    def listar_emprestimos_atrasados(self) -> List[Emprestimo]:
        return [e for e in self.emprestimos if e.esta_atrasado()]

    # ---------- as duas operacoes principais ----------

    def emprestar_livro(self, isbn: str, id_membro: int) -> Emprestimo:
        livro = self.livros.get(isbn)
        if livro is None:
            raise ValueError(f"Nao existe livro cadastrado com o ISBN {isbn}.")

        membro = self.membros.get(id_membro)
        if membro is None:
            raise ValueError(f"Nao existe membro cadastrado com o id {id_membro}.")

        # regra 1: nao pode emprestar livro que ja esta emprestado
        if self.livro_esta_emprestado(isbn):
            raise ValueError(f"O livro '{livro.titulo}' ja esta emprestado no momento.")

        # regra 2: cada membro tem um limite de emprestimos simultaneos
        ativos = self.emprestimos_ativos_do_membro(id_membro)
        if len(ativos) >= membro.limite_emprestimos:
            raise ValueError(
                f"{membro.nome} ja atingiu o limite de {membro.limite_emprestimos} "
                f"emprestimos simultaneos."
            )

        hoje = date.today()
        data_prevista = hoje + timedelta(days=PRAZO_EMPRESTIMO_DIAS)

        emprestimo = Emprestimo(
            id=self.proximo_id_emprestimo,
            isbn_livro=isbn,
            id_membro=id_membro,
            data_emprestimo=hoje.isoformat(),
            data_prevista_devolucao=data_prevista.isoformat(),
            data_devolucao=None,
        )
        self.emprestimos.append(emprestimo)
        self.proximo_id_emprestimo += 1
        return emprestimo

    def devolver_livro(self, id_emprestimo: int) -> Emprestimo:
        emprestimo = self._buscar_emprestimo(id_emprestimo)
        if emprestimo is None:
            raise ValueError(f"Nao existe emprestimo com o id {id_emprestimo}.")
        if emprestimo.foi_devolvido():
            raise ValueError("Esse emprestimo ja foi devolvido antes.")

        emprestimo.data_devolucao = date.today().isoformat()
        return emprestimo

    # ---------- persistencia ----------

    def salvar(self, caminho: str) -> None:
        dados = {
            "livros": [asdict(l) for l in self.livros.values()],
            "membros": [asdict(m) for m in self.membros.values()],
            "emprestimos": [asdict(e) for e in self.emprestimos],
            "proximo_id_membro": self.proximo_id_membro,
            "proximo_id_emprestimo": self.proximo_id_emprestimo,
        }
        with open(caminho, "w", encoding="utf-8") as arquivo:
            json.dump(dados, arquivo, ensure_ascii=False, indent=2)

    @classmethod
    def carregar(cls, caminho: str) -> "Biblioteca":
        biblioteca = cls()

        if not os.path.exists(caminho):
            return biblioteca

        try:
            with open(caminho, "r", encoding="utf-8") as arquivo:
                dados = json.load(arquivo)
        except (json.JSONDecodeError, OSError):
            # arquivo existe mas ta ilegivel: melhor comecar limpo do que travar o programa
            print("Aviso: nao consegui ler o arquivo de dados, comecando do zero.")
            return biblioteca

        # reconstruo cada objeto a partir do dicionario que veio do JSON
        for item in dados.get("livros", []):
            livro = Livro(**item)
            biblioteca.livros[livro.isbn] = livro

        for item in dados.get("membros", []):
            membro = Membro(**item)
            biblioteca.membros[membro.id] = membro

        for item in dados.get("emprestimos", []):
            emprestimo = Emprestimo(**item)
            biblioteca.emprestimos.append(emprestimo)

        biblioteca.proximo_id_membro = dados.get("proximo_id_membro", 1)
        biblioteca.proximo_id_emprestimo = dados.get("proximo_id_emprestimo", 1)
        return biblioteca
