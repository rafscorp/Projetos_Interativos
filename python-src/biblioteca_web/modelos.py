"""
Modelos do sistema de biblioteca.

Aqui ficam so as "entidades" (Livro, Membro, Emprestimo) com seus dados e
as regrinhas que dizem respeito só a elas mesmas (tipo calcular atraso de
UM emprestimo). Regra de negocio que envolve mais de uma entidade junta
(tipo "esse livro ja ta emprestado?") fica na classe Biblioteca, la no
biblioteca.py, porque ai sim precisa olhar a colecao inteira de dados.
"""

from dataclasses import dataclass
from datetime import date
from typing import Optional


@dataclass
class Livro:
    isbn: str
    titulo: str
    autor: str
    ano: int

    def __str__(self) -> str:
        return f"{self.titulo} ({self.ano}) - {self.autor} [ISBN {self.isbn}]"


@dataclass
class Membro:
    id: int
    nome: str
    limite_emprestimos: int = 3

    def __str__(self) -> str:
        return f"#{self.id} {self.nome} (limite: {self.limite_emprestimos} emprestimos)"


@dataclass
class Emprestimo:
    id: int
    isbn_livro: str
    id_membro: int
    data_emprestimo: str            # guardo em formato ISO "AAAA-MM-DD" pra facilitar salvar/comparar
    data_prevista_devolucao: str
    data_devolucao: Optional[str] = None  # None enquanto o livro nao volta

    def foi_devolvido(self) -> bool:
        return self.data_devolucao is not None

    def calcular_dias_atraso(self) -> int:
        # se ja foi devolvido, comparo a data real de devolucao com a prevista;
        # se ainda esta emprestado, comparo a data de hoje com a prevista.
        # assim a funcao serve tanto pra emprestimo em aberto quanto ja fechado
        prevista = date.fromisoformat(self.data_prevista_devolucao)
        referencia = (
            date.fromisoformat(self.data_devolucao)
            if self.foi_devolvido()
            else date.today()
        )
        dias = (referencia - prevista).days
        return max(dias, 0)  # se devolveu antes do prazo, nao existe atraso "negativo"

    def esta_atrasado(self) -> bool:
        return self.calcular_dias_atraso() > 0
