"""
Gerenciador de Tarefas CLI
--------------------------
Programa de linha de comando pra organizar tarefas do dia a dia.
Tudo fica salvo num arquivo JSON do lado do script, entao os dados
nao se perdem quando voce fecha o terminal.
"""

import json
import os
from dataclasses import dataclass, asdict
from typing import List, Optional

# uso o caminho absoluto do script (nao o diretorio atual) pra o arquivo
# de dados sempre ficar no mesmo lugar, nao importa de onde eu rode o programa
PASTA_SCRIPT = os.path.dirname(os.path.abspath(__file__))
ARQUIVO_DADOS = os.path.join(PASTA_SCRIPT, "tarefas.json")

PRIORIDADES_VALIDAS = ("baixa", "media", "alta")


@dataclass
class Tarefa:
    id: int
    descricao: str
    prioridade: str = "media"
    prazo: Optional[str] = None  # guardo como string "DD/MM/AAAA" pra simplificar, ou None se nao tiver
    concluida: bool = False


# ---------- persistencia ----------

def carregar_tarefas() -> List[Tarefa]:
    # se o arquivo nao existe ainda (primeira vez rodando o programa), comeca com lista vazia
    if not os.path.exists(ARQUIVO_DADOS):
        return []

    try:
        with open(ARQUIVO_DADOS, "r", encoding="utf-8") as arquivo:
            dados_brutos = json.load(arquivo)
    except (json.JSONDecodeError, OSError):
        # arquivo corrompido ou ilegivel: em vez de derrubar o programa,
        # aviso o usuario e comeca do zero
        print("Aviso: nao consegui ler o arquivo de tarefas, comecando com uma lista vazia.")
        return []

    tarefas = []
    for item in dados_brutos:
        # uso **item pra jogar as chaves do dicionario direto nos campos da dataclass
        tarefas.append(Tarefa(**item))
    return tarefas


def salvar_tarefas(tarefas: List[Tarefa]) -> None:
    # asdict transforma cada dataclass num dicionario comum, que o json sabe serializar
    dados = [asdict(t) for t in tarefas]
    with open(ARQUIVO_DADOS, "w", encoding="utf-8") as arquivo:
        json.dump(dados, arquivo, ensure_ascii=False, indent=2)


def proximo_id(tarefas: List[Tarefa]) -> int:
    # pego o maior id que ja existe e somo 1, assim nao repete id mesmo depois de remover tarefas
    if not tarefas:
        return 1
    return max(t.id for t in tarefas) + 1


# ---------- acoes do menu ----------

def acao_adicionar(tarefas: List[Tarefa]) -> None:
    descricao = input("Descricao da tarefa: ").strip()
    if not descricao:
        print("Descricao vazia, cancelei a operacao.")
        return

    prioridade = input("Prioridade (baixa/media/alta) [media]: ").strip().lower()
    if not prioridade:
        prioridade = "media"
    if prioridade not in PRIORIDADES_VALIDAS:
        print(f"Prioridade invalida, usando 'media'. (opcoes: {', '.join(PRIORIDADES_VALIDAS)})")
        prioridade = "media"

    prazo = input("Prazo (DD/MM/AAAA, ou deixe em branco): ").strip()
    prazo = prazo if prazo else None

    nova_tarefa = Tarefa(
        id=proximo_id(tarefas),
        descricao=descricao,
        prioridade=prioridade,
        prazo=prazo,
    )
    tarefas.append(nova_tarefa)
    salvar_tarefas(tarefas)
    print(f"Tarefa #{nova_tarefa.id} adicionada.")


def acao_listar(tarefas: List[Tarefa]) -> None:
    if not tarefas:
        print("Nenhuma tarefa cadastrada ainda.")
        return

    # ordeno por concluida (pendentes primeiro) e depois por prioridade,
    # pra quem abre a lista ja ver o que importa no topo
    ordem_prioridade = {"alta": 0, "media": 1, "baixa": 2}
    tarefas_ordenadas = sorted(
        tarefas,
        key=lambda t: (t.concluida, ordem_prioridade.get(t.prioridade, 1)),
    )

    print("\n--- Suas tarefas ---")
    for t in tarefas_ordenadas:
        status = "[X]" if t.concluida else "[ ]"
        prazo_str = f" | prazo: {t.prazo}" if t.prazo else ""
        print(f"{status} #{t.id} ({t.prioridade}) {t.descricao}{prazo_str}")
    print()


def buscar_tarefa_por_id(tarefas: List[Tarefa], id_tarefa: int) -> Optional[Tarefa]:
    for t in tarefas:
        if t.id == id_tarefa:
            return t
    return None


def ler_id_do_usuario(mensagem: str) -> Optional[int]:
    # centralizo a leitura de id aqui pra nao repetir o try/except em cada acao
    valor = input(mensagem).strip()
    try:
        return int(valor)
    except ValueError:
        print("Isso nao e um numero valido.")
        return None


def acao_concluir(tarefas: List[Tarefa]) -> None:
    if not tarefas:
        print("Nenhuma tarefa cadastrada ainda.")
        return

    id_tarefa = ler_id_do_usuario("ID da tarefa a marcar como concluida: ")
    if id_tarefa is None:
        return

    tarefa = buscar_tarefa_por_id(tarefas, id_tarefa)
    if tarefa is None:
        print(f"Nao achei nenhuma tarefa com id {id_tarefa}.")
        return

    if tarefa.concluida:
        print("Essa tarefa ja estava concluida.")
        return

    tarefa.concluida = True
    salvar_tarefas(tarefas)
    print(f"Tarefa #{tarefa.id} marcada como concluida.")


def acao_remover(tarefas: List[Tarefa]) -> None:
    if not tarefas:
        print("Nenhuma tarefa cadastrada ainda.")
        return

    id_tarefa = ler_id_do_usuario("ID da tarefa a remover: ")
    if id_tarefa is None:
        return

    tarefa = buscar_tarefa_por_id(tarefas, id_tarefa)
    if tarefa is None:
        print(f"Nao achei nenhuma tarefa com id {id_tarefa}.")
        return

    confirmacao = input(f"Remover '{tarefa.descricao}'? (s/n): ").strip().lower()
    if confirmacao != "s":
        print("Cancelado.")
        return

    tarefas.remove(tarefa)
    salvar_tarefas(tarefas)
    print("Tarefa removida.")


def acao_editar(tarefas: List[Tarefa]) -> None:
    if not tarefas:
        print("Nenhuma tarefa cadastrada ainda.")
        return

    id_tarefa = ler_id_do_usuario("ID da tarefa a editar: ")
    if id_tarefa is None:
        return

    tarefa = buscar_tarefa_por_id(tarefas, id_tarefa)
    if tarefa is None:
        print(f"Nao achei nenhuma tarefa com id {id_tarefa}.")
        return

    print("Deixe em branco pra manter o valor atual.")

    nova_prioridade = input(f"Nova prioridade [{tarefa.prioridade}]: ").strip().lower()
    if nova_prioridade:
        if nova_prioridade in PRIORIDADES_VALIDAS:
            tarefa.prioridade = nova_prioridade
        else:
            print(f"Prioridade invalida, mantive '{tarefa.prioridade}'.")

    prazo_atual = tarefa.prazo if tarefa.prazo else "sem prazo"
    novo_prazo = input(f"Novo prazo (DD/MM/AAAA) [{prazo_atual}]: ").strip()
    if novo_prazo:
        tarefa.prazo = novo_prazo

    salvar_tarefas(tarefas)
    print(f"Tarefa #{tarefa.id} atualizada.")


# ---------- menu principal ----------

def exibir_menu() -> None:
    print("=" * 40)
    print("GERENCIADOR DE TAREFAS")
    print("=" * 40)
    print("1 - Adicionar tarefa")
    print("2 - Listar tarefas")
    print("3 - Marcar tarefa como concluida")
    print("4 - Editar prioridade/prazo")
    print("5 - Remover tarefa")
    print("0 - Sair")


def main() -> None:
    tarefas = carregar_tarefas()

    # dicionario de acoes: fica mais facil de ler do que um monte de if/elif
    acoes = {
        "1": acao_adicionar,
        "2": acao_listar,
        "3": acao_concluir,
        "4": acao_editar,
        "5": acao_remover,
    }

    while True:
        exibir_menu()
        opcao = input("Escolha uma opcao: ").strip()

        if opcao == "0":
            print("Ate mais!")
            break

        acao = acoes.get(opcao)
        if acao is None:
            print("Opcao invalida, tenta de novo.\n")
            continue

        try:
            acao(tarefas)
        except KeyboardInterrupt:
            # se o usuario der Ctrl+C no meio de um input, nao quero que o
            # programa inteiro morra, so volta pro menu
            print("\nOperacao cancelada.")
        print()


if __name__ == "__main__":
    main()
