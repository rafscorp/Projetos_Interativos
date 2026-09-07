#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <emscripten.h>
#define LinhaBox "─"
#define LinhaNormal "-"
#define TamanhoLinha 60

// ponte assincrona com o JS: pausa a execucao do wasm (via ASYNCIFY) ate o
// usuario digitar uma linha no terminal e apertar Enter. Substitui o
// fgets(stdin), que so sabe ler de um stdin sincrono que nao existe de
// verdade no navegador.
EM_ASYNC_JS(void, web_read_line, (char *buf, int max_len), {
    const linha = await globalThis.__webReadLine();
    const bytes = new TextEncoder().encode(linha.slice(0, max_len - 1));
    HEAPU8.set(bytes, buf);
    HEAPU8[buf + bytes.length] = 0;
});

EM_JS(void, web_clear_screen, (), {
    if (globalThis.__webClearScreen) globalThis.__webClearScreen();
});

void limpartela()
{
    web_clear_screen();
}

void slep(int tempo)
{
    emscripten_sleep(tempo * 1000);
}

void linha(int tamanho, const char *tipo)
{
    for (int i = 0; i < tamanho; i++)
    {
        printf("%s", tipo);
    }
    printf("\n");
}

void cabecalho(const char *nome)
{
    int espacos = (TamanhoLinha - (int)strlen(nome)) / 2;
    if (espacos < 0) {
        espacos = 0;
    }

    linha(TamanhoLinha, LinhaBox);
    printf("%*s%s\n", espacos, "", nome);
    linha(TamanhoLinha, LinhaBox);
    printf("\n");
}

int lerInteiro(const char *impressao)
{
    char buffer[50];

    printf("%s\n", impressao);
    web_read_line(buffer, sizeof(buffer));

    return (int) strtol(buffer, NULL, 10);
}

//essa função devolve um ponteiro alocado no heap (malloc), não uma variável local -
//por isso quem chama lerChar() é responsável por dar free() depois de usar (o main.c faz isso certinho)
//se não desse free, cada leitura ia vazar um pouquinho de memória (memory leak) até o programa fechar
char* lerChar(const char *impressao)
{
    char buffer[50];

    printf("%s\n", impressao);
    web_read_line(buffer, sizeof(buffer));

    char *str = malloc(strlen(buffer) + 1); //+1 é pro caractere '\0' que fecha a string
    if (!str) {
        printf("Erro de memoria\n");
        slep(1);
        exit(1);
    }
    strcpy(str, buffer);

    return str;
}

void pausar() {
    char buf[8];
    printf("\nPressione ENTER para continuar...\n");
    web_read_line(buf, sizeof(buf));
}

bool verifica_nomeclatura(char nome[64])
{
    int temLetra = 0;

    if (strlen(nome) == 0) return false;

    for (int i = 0; i < strlen(nome); i++)
    {
        if (isalpha((unsigned char)nome[i])) {
            temLetra = 1;
        }
        else if (!isspace((unsigned char)nome[i])) {
            return false;
        }
    }
    return temLetra;
}

//valida preço no formato brasileiro tipo "1.234,56" olhando os caracteres de trás pra frente
//(mais fácil validar centavos e separador decimal começando do fim da string do que do início)
bool validar_preco(char *entrada) {
    int tam = strlen(entrada);
    if (tam < 4) return false;

    if (entrada[tam - 3] == '.') {
        entrada[tam - 3] = ',';
    }

    if (entrada[tam - 3] != ',' || !isdigit(entrada[tam - 2]) || !isdigit(entrada[tam - 1])) {
        return false;
    }

    int contador_digitos = 0;
    for (int i = tam - 4; i >= 0; i--) {
        if (isdigit(entrada[i])) {
            contador_digitos++;
        } else if (entrada[i] == '.') {
            if (contador_digitos != 3) return false;
            contador_digitos = 0;
        } else {
            return false;
        }
    }
    return (contador_digitos > 0);
}

bool verifica_cpf(char *cpf)
{
    if (strlen(cpf) != 11) return false;
    for (int i = 0; i < 11; i++)
    {
        if (!isdigit(cpf[i])) return false;
    }
    return true;
}

//struct é como criar um "tipo novo" que junta vários dados relacionados numa coisa só
//aqui em vez de ter um array de nomes e outro array de preços (desconectados), cada Produtos já
//carrega os dois campos juntos - fica impossível o preço do índice 3 ser de outro produto por engano
typedef struct
{
    char preco[24];
    char produto[50];
} Produtos;

typedef struct
{
    char nome[64];
    char cpf[24];
} Clientes;

//esses dois ponteiros começam NULL (lista vazia) e crescem com realloc conforme cadastra gente/produto
//é basicamente um array de tamanho dinâmico: só ocupa memória do que realmente existe cadastrado
Produtos *produtos = NULL;
int total_produtos = 0;

Clientes *clientes = NULL;
int total_clientes = 0;

void print_centralizado(const char *texto, int largura)
{
    int len = strlen(texto);
    int espacos = (largura - len) / 2;

    if (espacos < 0) espacos = 0;

    printf("%*s%s%*s",
           espacos, "",
           texto,
           largura - len - espacos, "");
}

bool listar_produto()
{
    if (produtos == NULL)
    {
        printf("Erro, nao tem nada na LISTA de Produtos\n");
        emscripten_sleep(3000);
        return false;
    }

    linha(TamanhoLinha, LinhaBox);

    printf("│");
    print_centralizado("Nome do Produto", 30);
    printf("│");
    print_centralizado("  Preço do Produto  ", 20);
    printf("\n");

    linha(TamanhoLinha, LinhaBox);
    for (int i = 0; i < total_produtos; i++)
    {
        printf("│");
        printf("ID %d", i+1);
        print_centralizado(produtos[i].produto, 30);
        printf("│");
        print_centralizado(produtos[i].preco, 20);
        printf("\n");
    }

    linha(TamanhoLinha, LinhaBox);
    return true;
}

bool listar_clientes()
{
    if (clientes == NULL)
    {
        printf("Erro, nao tem nada na LISTA de Produtos\n");
        emscripten_sleep(3000);
        return false;
    }

    linha(TamanhoLinha, LinhaBox);

    printf("│");
    print_centralizado("Nome dos Usuários", 30);
    printf(" │");
    print_centralizado("   CPF dos Usuários  ", 20);
    printf("\n");

    linha(TamanhoLinha, LinhaBox);
    for (int i = 0; i < total_clientes; i++)
    {
        printf("│");
        printf("ID %d", i+1);
        print_centralizado(clientes[i].nome, 30);
        printf("│");
        print_centralizado(clientes[i].cpf, 20);
        printf("\n");
    }

    linha(TamanhoLinha, LinhaBox);
    return true;
}

bool cpf_ja_existe(const char *cpf)
{
    for (int i = 0; i < total_clientes; i++)
    {
        if (strcmp(clientes[i].cpf, cpf) == 0)
        {
            return true;
        }
    }
    return false;
}

bool Nome_existe_cliente(const char *nome)
{
    for (int i = 0; i < total_clientes; i++)
    {
        if (strcmp(clientes[i].nome, nome) == 0)
        {
            return true;
        }
    }
    return false;
}

//deixa o nome em minúsculo e remove espaços duplicados só pra COMPARAR (ex: "João  Silva" vira "joão silva")
//assim "João Silva" e "joão  silva" são detectados como o mesmo cliente na hora de checar duplicidade
//usa dois índices (i lê o original, j escreve o resultado) pra montar a string nova por cima da mesma memória
void normalizar_nome(char *str)
{
    int i = 0, j = 0;
    bool espaco = false;

    while (str[i])
    {
        if (isspace((unsigned char)str[i]))
        {
            if (!espaco)
            {
                str[j++] = ' ';
                espaco = true;
            }
        }
        else
        {
            str[j++] = tolower((unsigned char)str[i]);
            espaco = false;
        }
        i++;
    }

    if (j > 0 && str[j - 1] == ' ')
        j--;

    str[j] = '\0';
}

bool adicionar_Clientes(char *nome, char *cpf)
{
    if (cpf_ja_existe(cpf))
    {
        printf("Erro: CPF ja cadastrado!\n");
        slep(2);
        return false;
    }

    char nome_norm[64];
    strcpy(nome_norm, nome);
    normalizar_nome(nome_norm);

    for (int i = 0; i < total_clientes; i++)
    {
        char existente[64];
        strcpy(existente, clientes[i].nome);
        normalizar_nome(existente);

        if (strcmp(existente, nome_norm) == 0)
        {
            printf("Erro: Cliente com esse nome ja existe!\n");
            slep(2);
            return false;
        }
    }

    //realloc pede pro sistema aumentar o bloco de memória em +1 struct
    //ele PODE mover o bloco inteiro de lugar na memória, por isso guardamos o resultado num ponteiro
    //temporário (tmp) antes: se der NULL (falhou), ainda temos o "clientes" antigo intacto pra não perder tudo
    Clientes *tmp = realloc(clientes, (total_clientes + 1) * sizeof(Clientes));
    if (!tmp)
    {
        printf("Erro ao alocar memoria!\n");
        slep(2);
        return false;
    }

    clientes = tmp;
    strcpy(clientes[total_clientes].nome, nome);
    strcpy(clientes[total_clientes].cpf, cpf);
    total_clientes++;

    return true;
}

bool adicionar_Produto(char *nome, char *preco)
{
    char nome_norm[50];
    strcpy(nome_norm, nome);
    normalizar_nome(nome_norm);

    for (int i = 0; i < total_produtos; i++)
    {
        char existente[50];
        strcpy(existente, produtos[i].produto);
        normalizar_nome(existente);

        if (strcmp(existente, nome_norm) == 0)
        {
            printf("Erro: Produto ja cadastrado!\n");
            slep(2);
            return false;
        }
    }

    Produtos *tmp = realloc(produtos, (total_produtos + 1) * sizeof(Produtos));
    if (!tmp)
    {
        printf("Erro ao alocar memoria!\n");
        slep(2);
        return false;
    }

    produtos = tmp;
    strcpy(produtos[total_produtos].produto, nome);
    strcpy(produtos[total_produtos].preco, preco);
    total_produtos++;

    return true;
}

//não existe "buraco" num array em C, então pra remover o item do meio a gente desloca
//todo mundo depois dele uma posição pra trás (ex: remove o [2], o que era [3] vira [2], o [4] vira [3]...)
//só depois de fechar o buraco é que dá pra diminuir o tamanho de verdade com realloc
void remover_produto(int id)
{
    if (produtos == NULL || total_produtos == 0)
    {
        printf("Lista vazia.\n");
        slep(2);
        return;
    }

    if (id < 0 || id >= total_produtos)
    {
        printf("ID invalido.\n");
        slep(2);
        return;
    }

    for (int i = id; i < total_produtos - 1; i++)
    {
        produtos[i] = produtos[i + 1];
    }

    total_produtos--;

    if (total_produtos == 0)
    {
        free(produtos);
        produtos = NULL;
    }
    else
    {
        Produtos *tmp = realloc(produtos, total_produtos * sizeof(Produtos));
        if (tmp != NULL)
            produtos = tmp;
    }

    printf("Produto removido.\n");
    slep(2);
}

void remover_cliente(int id)
{
    if (clientes == NULL || total_clientes == 0)
    {
        printf("Lista vazia.\n");
        slep(2);
        return;
    }

    if (id < 0 || id >= total_clientes)
    {
        printf("ID invalido.\n");
        slep(2);
        return;
    }

    for (int i = id; i < total_clientes - 1; i++)
    {
        clientes[i] = clientes[i + 1];
    }

    total_clientes--;

    if (total_clientes == 0)
    {
        free(clientes);
        clientes = NULL;
    }
    else
    {
        Clientes *tmp = realloc(clientes, total_clientes * sizeof(Clientes));
        if (tmp != NULL)
            clientes = tmp;
    }

    printf("Cliente removido.\n");
    slep(2);
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    char *Nomeproduto;
    char *Nomecliente;
    char *preco;
    char *cpf;

    int cont = 1;
    while (cont == 1)
    {
        limpartela();
        cabecalho("Mercado Assaí Produtos");
        printf("  1 - Listar Produtos    \n");
        printf("  2 - Listar Clientes    \n");
        printf("  3 - Adicionar Produto  \n");
        printf("  4 - Cadastrar Cliente  \n");
        printf("  5 - Remover Produto    \n");
        printf("  6 - Remover Cliente    \n");
        printf("  7 - Sair             \n\n");

        int op = lerInteiro("Escolha: ");

        switch (op)
        {
        case 1:
            listar_produto();
            pausar();
            break;

        case 2:
            listar_clientes();
            pausar();
            break;

        case 3:
            {
                bool nome_ok;
                bool preco_ok;

                do
                {
                    Nomeproduto = lerChar("Nome do produto: ");
                    nome_ok = verifica_nomeclatura(Nomeproduto);

                    if (!nome_ok)
                    {
                        printf("Erro no nome do produto\n");
                        free(Nomeproduto);
                        Nomeproduto = NULL;
                    }

                } while (!nome_ok);

                do
                {
                    preco = lerChar("Preco: ");
                    preco_ok = validar_preco(preco);

                    if (!preco_ok)
                    {
                        printf("Preço inválido\n");
                        free(preco);
                        preco = NULL;
                    }

                } while (!preco_ok);

                if (adicionar_Produto(Nomeproduto, preco))
                {
                    printf("Produto adicionado com sucesso! = [%s][R$ %s]\n",
                           Nomeproduto, preco);
                    slep(3);
                }
                else
                {
                    printf("Erro ao adicionar produto\n");
                }

                free(Nomeproduto);
                free(preco);
                break;
            }

        case 4:
            {
                bool nome_ok;
                bool cpf_ok;

                do
                {
                    Nomecliente = lerChar("Nome do cliente: ");
                    nome_ok = verifica_nomeclatura(Nomecliente);

                    if (!nome_ok)
                    {
                        printf("Erro no nome\n");
                        free(Nomecliente);
                        Nomecliente = NULL;
                    }

                } while (!nome_ok);

                do
                {
                    cpf = lerChar("CPF: ");
                    cpf_ok = verifica_cpf(cpf);

                    if (!cpf_ok)
                    {
                        printf("CPF inválido\n");
                        free(cpf);
                        cpf = NULL;
                    }

                } while (!cpf_ok);

                if (adicionar_Clientes(Nomecliente, cpf))
                {
                    printf("Pessoa cadastrada com sucesso! = [%s][%s]\n",
                           Nomecliente, cpf);
                    slep(3);
                }

                free(Nomecliente);
                free(cpf);
                break;
            }

        case 5:
            {
                if (listar_produto())
                {
                    int id = lerInteiro("ID do produto para remover: ") - 1;
                    remover_produto(id);
                    pausar();
                }
                break;
            }

        case 6:
            {
                if (listar_clientes())
                {
                    int id = lerInteiro("ID do cliente para remover: ")-1;
                    remover_cliente(id);
                    pausar();
                }
                break;
            }
        case 7:
            printf("Saindo\n");
            fflush(stdout);
            slep(3);
            cont = 0;
            break;

        default:
            printf(" [ID][%d] id Inválido \n", op);
            fflush(stdout);
            slep(3);
        }
    }

    free(produtos);
    free(clientes);
    return 0;
}
