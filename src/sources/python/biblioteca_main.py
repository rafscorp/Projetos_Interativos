"""
Sistema de Biblioteca - ponto de entrada.

Aqui so tem a interface de terminal (menu, input, print). Toda a regra de
negocio de verdade mora na classe Biblioteca (biblioteca.py), e as
entidades ficam em modelos.py. Separei em 3 arquivos assim porque cada um
tem uma responsabilidade bem diferente: dado puro, regra de negocio, e
interface com o usuario. Se um dia eu quiser trocar o menu de terminal por
uma interface web, so preciso mexer nesse arquivo aqui.
"""

import os

from biblioteca import Biblioteca

PASTA_SCRIPT = os.path.dirname(os.path.abspath(__file__))
ARQUIVO_DADOS = os.path.join(PASTA_SCRIPT, "biblioteca_dados.json")


# ---------- funcoes auxiliares de leitura de input ----------

def ler_inteiro(mensagem: str):
    valor = input(mensagem).strip()
    try:
        return int(valor)
    except ValueError:
        print("Isso nao e um numero inteiro valido.")
        return None


# ---------- acoes do menu ----------

def acao_cadastrar_livro(biblioteca: Biblioteca) -> None:
    isbn = input("ISBN: ").strip()
    titulo = input("Titulo: ").strip()
    autor = input("Autor: ").strip()
    ano = ler_inteiro("Ano de publicacao: ")
    if ano is None:
        return

    try:
        livro = biblioteca.cadastrar_livro(isbn, titulo, autor, ano)
    except ValueError as erro:
        print(f"Erro: {erro}")
        return

    print(f"Livro cadastrado: {livro}")


def acao_cadastrar_membro(biblioteca: Biblioteca) -> None:
    nome = input("Nome do membro: ").strip()
    texto_limite = input("Limite de emprestimos simultaneos [3]: ").strip()

    limite = 3
    if texto_limite:
        try:
            limite = int(texto_limite)
        except ValueError:
            print("Limite invalido, usando o padrao (3).")
            limite = 3

    try:
        membro = biblioteca.cadastrar_membro(nome, limite)
    except ValueError as erro:
        print(f"Erro: {erro}")
        return

    print(f"Membro cadastrado: {membro}")


def acao_emprestar(biblioteca: Biblioteca) -> None:
    if not biblioteca.livros:
        print("Nenhum livro cadastrado ainda.")
        return
    if not biblioteca.membros:
        print("Nenhum membro cadastrado ainda.")
        return

    isbn = input("ISBN do livro: ").strip()
    id_membro = ler_inteiro("ID do membro: ")
    if id_membro is None:
        return

    try:
        emprestimo = biblioteca.emprestar_livro(isbn, id_membro)
    except ValueError as erro:
        print(f"Erro: {erro}")
        return

    print(
        f"Emprestimo #{emprestimo.id} registrado. "
        f"Devolucao prevista para {emprestimo.data_prevista_devolucao}."
    )


def acao_devolver(biblioteca: Biblioteca) -> None:
    if not biblioteca.emprestimos:
        print("Nenhum emprestimo registrado ainda.")
        return

    id_emprestimo = ler_inteiro("ID do emprestimo: ")
    if id_emprestimo is None:
        return

    try:
        emprestimo = biblioteca.devolver_livro(id_emprestimo)
    except ValueError as erro:
        print(f"Erro: {erro}")
        return

    if emprestimo.esta_atrasado():
        dias = emprestimo.calcular_dias_atraso()
        print(f"Livro devolvido com {dias} dia(s) de atraso.")
    else:
        print("Livro devolvido dentro do prazo.")


def acao_listar_livros(biblioteca: Biblioteca) -> None:
    if not biblioteca.livros:
        print("Nenhum livro cadastrado ainda.")
        return

    print("\n--- Acervo ---")
    for livro in biblioteca.livros.values():
        situacao = "emprestado" if biblioteca.livro_esta_emprestado(livro.isbn) else "disponivel"
        print(f"{livro} - {situacao}")
    print()


def acao_listar_membros(biblioteca: Biblioteca) -> None:
    if not biblioteca.membros:
        print("Nenhum membro cadastrado ainda.")
        return

    print("\n--- Membros ---")
    for membro in biblioteca.membros.values():
        ativos = len(biblioteca.emprestimos_ativos_do_membro(membro.id))
        print(f"{membro} - {ativos} emprestimo(s) ativo(s) no momento")
    print()


def acao_listar_emprestimos(biblioteca: Biblioteca) -> None:
    if not biblioteca.emprestimos:
        print("Nenhum emprestimo registrado ainda.")
        return

    print("\n--- Emprestimos ---")
    for e in biblioteca.emprestimos:
        livro = biblioteca.livros.get(e.isbn_livro)
        membro = biblioteca.membros.get(e.id_membro)
        titulo = livro.titulo if livro else "(livro removido)"
        nome_membro = membro.nome if membro else "(membro removido)"

        if e.foi_devolvido():
            status = f"devolvido em {e.data_devolucao}"
        elif e.esta_atrasado():
            status = f"EM ABERTO, atrasado ha {e.calcular_dias_atraso()} dia(s)"
        else:
            status = f"em aberto, previsto para {e.data_prevista_devolucao}"

        print(f"#{e.id} '{titulo}' com {nome_membro} - {status}")
    print()


def acao_relatorio_atrasados(biblioteca: Biblioteca) -> None:
    atrasados = biblioteca.listar_emprestimos_atrasados()
    if not atrasados:
        print("Nenhum emprestimo atrasado no momento. Tudo em dia!")
        return

    print("\n--- Emprestimos atrasados ---")
    for e in atrasados:
        livro = biblioteca.livros.get(e.isbn_livro)
        membro = biblioteca.membros.get(e.id_membro)
        titulo = livro.titulo if livro else "(livro removido)"
        nome_membro = membro.nome if membro else "(membro removido)"
        print(f"#{e.id} '{titulo}' com {nome_membro} - {e.calcular_dias_atraso()} dia(s) de atraso")
    print()


# ---------- menu principal ----------

def exibir_menu() -> None:
    print("=" * 45)
    print("SISTEMA DE BIBLIOTECA")
    print("=" * 45)
    print("1 - Cadastrar livro")
    print("2 - Cadastrar membro")
    print("3 - Emprestar livro")
    print("4 - Devolver livro")
    print("5 - Listar acervo")
    print("6 - Listar membros")
    print("7 - Listar emprestimos")
    print("8 - Relatorio de atrasados")
    print("0 - Sair")


def main() -> None:
    biblioteca = Biblioteca.carregar(ARQUIVO_DADOS)

    acoes = {
        "1": acao_cadastrar_livro,
        "2": acao_cadastrar_membro,
        "3": acao_emprestar,
        "4": acao_devolver,
        "5": acao_listar_livros,
        "6": acao_listar_membros,
        "7": acao_listar_emprestimos,
        "8": acao_relatorio_atrasados,
    }

    while True:
        exibir_menu()
        opcao = input("Escolha uma opcao: ").strip()

        if opcao == "0":
            biblioteca.salvar(ARQUIVO_DADOS)
            print("Dados salvos. Ate mais!")
            break

        acao = acoes.get(opcao)
        if acao is None:
            print("Opcao invalida, tenta de novo.\n")
            continue

        try:
            acao(biblioteca)
            # salvo depois de cada acao, assim se o programa fechar de
            # jeito inesperado (ex: o usuario fecha o terminal) o maximo
            # que se perde e a ultima operacao, nunca o historico todo
            biblioteca.salvar(ARQUIVO_DADOS)
        except KeyboardInterrupt:
            print("\nOperacao cancelada.")
        print()


if __name__ == "__main__":
    main()
