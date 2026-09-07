#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ================== PLATAFORMA ================== */
#ifdef _WIN32
#include <windows.h>
#include <direct.h>

#define CLEAR_SCREEN() system("cls")
#define SLEEP_MS(ms) Sleep(ms)
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>

#define CLEAR_SCREEN() system("clear")
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define MKDIR(path) mkdir(path, 0777)
#endif
/* ================================================ */
//essas macros escondem a diferença entre Windows e Linux atrás de um nome só
//CLEAR_SCREEN(), SLEEP_MS(ms) e MKDIR(path) chamam a função certa dependendo do _WIN32
//assim o resto do código nem precisa saber em qual sistema está rodando

/* ================== CONSTANTES DE CORES ================== */
#define RST "\x1b[0m"
#define BOLD "\x1b[1m"
#define ITA "\x1b[3m"
#define UDL "\x1b[4m"
#define REV "\x1b[7m"

#define BLK "\x1b[30m"
#define RED "\x1b[31m"
#define GRN "\x1b[32m"
#define YEL "\x1b[33m"
#define BLU "\x1b[34m"
#define MAG "\x1b[35m"
#define CYN "\x1b[36m"
#define WHT "\x1b[37m"
#define GRY "\x1b[90m"

#define BLK_B "\x1b[90m"
#define RED_B "\x1b[91m"
#define GRN_B "\x1b[92m"
#define YEL_B "\x1b[93m"
#define BLU_B "\x1b[94m"
#define MAG_B "\x1b[95m"
#define CYN_B "\x1b[96m"
#define WHT_B "\x1b[97m"
#define GRY_B "\x1b[37m"

#define BLK_BK "\x1b[40m"
#define RED_BK "\x1b[41m"
#define GRN_BK "\x1b[42m"
#define YEL_BK "\x1b[43m"
#define BLU_BK "\x1b[44m"
#define MAG_BK "\x1b[45m"
#define CYN_BK "\x1b[46m"
#define WHT_BK "\x1b[47m"
#define GRY_BK "\x1b[100m"

#define BLK_B_BK "\x1b[100m"
#define RED_B_BK "\x1b[101m"
#define GRN_B_BK "\x1b[102m"
#define YEL_B_BK "\x1b[103m"
#define BLU_B_BK "\x1b[104m"
#define MAG_B_BK "\x1b[105m"
#define CYN_B_BK "\x1b[106m"
#define WHT_B_BK "\x1b[107m"
#define GRY_B_BK "\x1b[47m"
//esse bloco todo é só código de escape ANSI (\x1b[...m), o terminal lê esses códigos
//e muda cor/fundo/negrito do texto que vier depois, até aparecer o RST (reset)
//separei em 4 grupos: cor de texto normal, cor de texto "brilhante" (_B), fundo (_BK) e fundo brilhante (_B_BK)

/* ================== ESTRUTURA DE LOGS ================== */
typedef enum
{
    MOD_SISTEMA,
    MOD_CALC_CIENTIFICA,
    MOD_CALC_UNI
} ModuloLog;

static char LOG_SESSAO[64]; //guarda o "nome" dessa sessão (data+hora), usado no nome dos arquivos de log

void Log_GerarSessao(void)
{
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    if (!t)
        return;
    strftime(LOG_SESSAO, sizeof(LOG_SESSAO), "%Y-%m-%d_%H-%M-%S", t);
}
//gera um timestamp tipo 2026-01-03_22-04-53 e guarda em LOG_SESSAO
//todo log escrito na mesma execução do programa cai no mesmo arquivo, porque usa esse mesmo nome

void Log_CriarDir(const char *path)
{
    if (MKDIR(path) != 0 && errno != EEXIST)
    {
        perror("Erro ao criar diretorio de log");
    }
}

void Log_Init(void)
{
    Log_GerarSessao();
    Log_CriarDir("logs");
    Log_CriarDir("logs/sistema");
    Log_CriarDir("logs/calculadora-cientifica");
    Log_CriarDir("logs/calculadora-cientifica/operacoes");
    Log_CriarDir("logs/calculadora-cientifica/erros");
    Log_CriarDir("logs/calculadora-uni");
    Log_CriarDir("logs/calculadora-uni/operacoes");
    Log_CriarDir("logs/calculadora-uni/erros");
}

void Log_Timestamp(char *buf, size_t tam)
{
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    if (!t)
        return;
    strftime(buf, tam, "%Y-%m-%d %H:%M:%S", t);
}

void Log_Escrever(ModuloLog modulo, int ehErro, const char *codigo, const char *descricao)
{
    char caminho[256];
    char linha[512];
    char tempo[32];
    Log_Timestamp(tempo, sizeof(tempo));

    if (modulo == MOD_SISTEMA)
    {
        snprintf(caminho, sizeof(caminho), "logs/sistema/%s.log", LOG_SESSAO);
    }
    else
    {
        const char *base = (modulo == MOD_CALC_CIENTIFICA) ? "calculadora-cientifica" : "calculadora-uni";
        const char *tipo = ehErro ? "erros" : "operacoes";
        snprintf(caminho, sizeof(caminho), "logs/%s/%s/%s.log", base, tipo, LOG_SESSAO);
    }

    FILE *f = fopen(caminho, "a"); //"a" = append, cada chamada abre, escreve uma linha e fecha - não fica com o arquivo preso
    if (!f)
        return;

    snprintf(linha, sizeof(linha), "[%s] [%s] [%s] [%s] -> %s",
             tempo,
             modulo == MOD_CALC_CIENTIFICA ? "CALC_CIENTIFICA" : modulo == MOD_CALC_UNI ? "CALC_UNI"
                                                                                        : "SISTEMA",
             ehErro ? "ERRO" : "OPERACAO",
             codigo,
             descricao);

    fprintf(f, "%s\n", linha);
    fclose(f);
}
//Log_Escrever é a função "motor": monta o caminho certo (sistema/cientifica/uni, operacao/erro)
//e escreve a linha formatada. as 3 de baixo são só atalhos pra não ter que montar os parâmetros toda hora
void Log_Sistema(const char *msg) { Log_Escrever(MOD_SISTEMA, 0, "INFO", msg); }
void Log_Operacao(ModuloLog mod, const char *op, const char *desc) { Log_Escrever(mod, 0, op, desc); }
void Log_Erro(ModuloLog mod, const char *cod, const char *desc) { Log_Escrever(mod, 1, cod, desc); }

/* ================== FUNÇÕES DE INTERFACE ================== */
const char *getCor(int cor)
{
    switch (cor)
    {
    case 1:
        return BLK;
    case 2:
        return RED;
    case 3:
        return GRN;
    case 4:
        return YEL;
    case 5:
        return BLU;
    case 6:
        return MAG;
    case 7:
        return CYN;
    case 8:
        return WHT;
    case 9:
        return BLK_B;
    case 10:
        return RED_B;
    case 11:
        return GRN_B;
    case 12:
        return YEL_B;
    case 13:
        return BLU_B;
    case 14:
        return MAG_B;
    case 15:
        return CYN_B;
    case 16:
        return WHT_B;
    case 17:
        return BLK_BK;
    case 18:
        return RED_BK;
    case 19:
        return GRN_BK;
    case 20:
        return YEL_BK;
    case 21:
        return BLU_BK;
    case 22:
        return MAG_BK;
    case 23:
        return CYN_BK;
    case 24:
        return WHT_BK;
    case 25:
        return BLK_B_BK;
    case 26:
        return RED_B_BK;
    case 27:
        return GRN_B_BK;
    case 28:
        return YEL_B_BK;
    case 29:
        return BLU_B_BK;
    case 30:
        return MAG_B_BK;
    case 31:
        return CYN_B_BK;
    case 32:
        return WHT_B_BK;
    default:
        return "";
    }
}

void LimpaTela(void) { CLEAR_SCREEN(); }

void slep(float tempo)
{
    if (tempo <= 0.0f)
        return;
    SLEEP_MS((int)(tempo * 1000));
}

void Linha(int tam, int tipo, int cor)
{
    const char *corStr = getCor(cor);
    const char *simbolo = (tipo == 0 ? "==" : "──");
    for (int i = 0; i < tam; i++)
        printf("%s" BOLD "%s" RST, corStr, simbolo);
    printf("\n");
}

void Cabecalho(const char *title, int tipo)
{
    if (tipo == 0)
    {
        Linha(39, 0, 1);
        printf(BOLD ITA GRN_B BLK_B_BK "%s\n" RST, title);
        Linha(39, 0, 1);
    }
    else if (tipo == 1)
    {
        Linha(39, 0, 1);
        printf(BOLD ITA CYN "            %s\n" RST, title);
    }
}

void Resposta(float valor)
{
    printf("\n      " BOLD BLK_B_BK " < RESPOSTA > " RST BOLD BLU_B_BK "  %.1f  " RST "\n" RST, valor);
}

void ClearInputUntilNewline(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}
//essa função existe pra resolver um problema clássico do scanf: se a pessoa digita "abc" onde
//era esperado um número, o scanf falha mas deixa o "abc\n" parado no buffer de entrada
//sem limpar isso, a próxima leitura já pegaria o lixo que sobrou e ia falhar de novo sem nem perguntar nada

int LerInt(const char *msg)
{
    int v;
    while (1)
    {
        if (msg)
        {
            printf(BOLD CYN_B_BK "%s" RST, msg);
            printf(" ");
        }
        if (scanf("%d", &v) == 1)
        {
            ClearInputUntilNewline();
            return v;
        }
        printf("Entrada invalida! Digite um numero inteiro: ");
        ClearInputUntilNewline();
    }
}

double LerDouble(const char *msg)
{
    double v;
    while (1)
    {
        if (msg)
        {
            printf(BOLD BLU_B_BK "%s" RST, msg);
            printf(" ");
            fflush(stdout);
        }
        if (scanf("%lf", &v) == 1)
        {
            ClearInputUntilNewline();
            return v;
        }
        printf(RED "Entrada invalida! Digite um numero: " RST);
        ClearInputUntilNewline();
    }
}

float LerFloat(const char *msg)
{
    float v;
    while (1)
    {
        if (msg)
            printf("%s", msg);
        if (scanf("%f", &v) == 1)
        {
            ClearInputUntilNewline();
            return v;
        }
        printf("Entrada invalida! Digite um numero: ");
        ClearInputUntilNewline();
    }
}

char LerCharSN(const char *msg)
{
    char c;
    while (1)
    {
        if (msg)
            printf("%s", msg);
        if (scanf(" %c", &c) == 1)
        {
            ClearInputUntilNewline();
            if (c == 'S' || c == 's')
                return 'S';
            if (c == 'N' || c == 'n')
                return 'N';
        }
        else
            ClearInputUntilNewline();
        printf("Valor invalido. Digite apenas S ou N.\n");
    }
}

char LerCharOpcao(const char *msg, const char *opcoes)
{
    char c;
    while (1)
    {
        if (msg)
            printf("%s", msg);
        if (scanf(" %c", &c) == 1)
        {
            ClearInputUntilNewline();
            for (size_t i = 0; opcoes[i] != '\0'; ++i)
            {
                //compara com a letra digitada E com a versão maiuscula/minuscula dela
                //(+-32 na tabela ASCII troca entre maiúscula e minúscula), assim aceita "r" ou "R" por exemplo
                if (c == opcoes[i] || c == (opcoes[i] >= 'a' && opcoes[i] <= 'z' ? opcoes[i] - 32 : opcoes[i] + 32))
                {
                    return c;
                }
            }
        }
        else
            ClearInputUntilNewline();
        printf("Entrada invalida. Opcoes validas: %s\n", opcoes);
    }
}

/* ================== LÓGICA MATEMÁTICA ================== */
double AdiC(double a, double b) { return a + b; }
double SubC(double a, double b) { return a - b; }
double MultC(double a, double b) { return a * b; }
double DivC(double a, double b)
{
    if (b == 0.0)
    {
        Log_Erro(MOD_CALC_CIENTIFICA, "DIV0", "Tentativa de divisao por zero");
        return 0.0;
    }
    return a / b;
}
double PorC(double a, double b) { return (a * b) / 100.0; }
double RaizC(double a)
{
    if (a < 0.0)
    {
        Log_Erro(MOD_CALC_CIENTIFICA, "RAIZ_NEG", "Tentativa de raiz de numero negativo");
        return 0.0;
    }
    return sqrt(a);
}
double PotC(double a, double b) { return pow(a, b); }
//esse bloco (C) e o de baixo (U) fazem basicamente a mesma coisa - dá pra perceber a repetição
//uma versão mais avançada resolveria isso com um vetor de ponteiro de função (função só troca de "qual conta fazer"),
//mas deixei repetido de propósito aqui porque fica mais fácil de ler linha por linha nesse ponto do aprendizado

double AdiU(double a, double b) { return a + b; }
double SubU(double a, double b) { return a - b; }
double MultiU(double a, double b) { return a * b; }
double DiviU(double a, double b)
{
    if (b == 0.0)
    {
        Log_Erro(MOD_CALC_UNI, "DIV0", "Tentativa de divisao por zero");
        return 0.0;
    }
    return a / b;
}

/* Controller para Computação Científica */
double CalcularOperacaoC(int op, double n1, double n2)
{
    double res = n1;
    char desc[128];
    if (op == 1)
    {
        res = AdiC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "ADICAO", "Soma cientifica realizada");
    }
    else if (op == 2)
    {
        res = SubC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "SUBTRACAO", "Subtracao cientifica realizada");
    }
    else if (op == 3)
    {
        res = MultC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "MULTIPLICACAO", "Multiplicacao cientifica realizada");
    }
    else if (op == 4)
    {
        res = DivC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "DIVISAO", "Divisao cientifica realizada");
    }
    else if (op == 5)
    {
        res = PorC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "PORCENTAGEM", "Calculo de porcentagem realizado");
    }
    else if (op == 6)
    {
        res = RaizC(n1);
        Log_Operacao(MOD_CALC_CIENTIFICA, "RAIZ", "Raiz quadrada realizada");
    }
    else if (op == 7)
    {
        res = PotC(n1, n2);
        Log_Operacao(MOD_CALC_CIENTIFICA, "POTENCIA", "Potenciacao realizada");
    }
    return res;
}

//essa função funciona como uma pequena "máquina de estados": estado 0 é a primeira conta (pede os 2 números),
//estado 1 é o modo de "encadear" contas em cima do resultado anterior (só pede um número novo por vez)
//esse padrão de "int estado" + while(1) é uma forma simples de controlar telas/fluxos diferentes na mesma função
void CalculadoraC(void)
{
    double num1 = 0.0, num2 = 0.0, resposta = 0.0;
    int op = 0;
    int estado = 0;
    const char *OP[] = {
        BOLD "     <  " CYN_BK " -- " BLU_BK " Adição          " BOLD MAG_BK " [  +  ]" BOLD GRN_BK "    --ID = 1",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Subtração       " BOLD MAG_BK " [  -  ]" BOLD GRN_BK "    --ID = 2",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Multiplicação   " BOLD MAG_BK " [  x  ]" BOLD GRN_BK "    --ID = 3",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Divisão         " BOLD MAG_BK " [  ÷  ]" BOLD GRN_BK "    --ID = 4",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Porcentagem     " BOLD MAG_BK " [  %  ]" BOLD GRN_BK "    --ID = 5",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Raiz Quadrada   " BOLD MAG_BK " [  2√ ]" BOLD GRN_BK "    --ID = 6",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Potenciação     " BOLD MAG_BK " [  3² ]" BOLD GRN_BK "    --ID = 7",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Saída           " BOLD MAG_BK " [ ExT ]" BOLD GRN_BK "    --ID = 8",
    };

    while (1)
    {
        if (estado == 0)
        {
            Cabecalho("Calculadora Científica", 0);
            printf("\n");
            num1 = LerDouble(" < Número > ");
            printf("\n" BLK_B_BK " < Operações Permitidas > " RST "\n");
            Linha(30, 1, 0);
            for (int i = 0; i < 8; ++i)
            {
                printf(" %s " RST BOLD "  >  "
                       "\n" RST,
                       OP[i]);
                slep(0.05f);
            }
            Linha(30, 1, 0);
            printf("\n");
            op = LerInt(" < Operador > ");
            if (op == 6)
            {
                estado = 1;
                resposta = CalcularOperacaoC(op, num1, 0);
            }
            else if ((op >= 1 && op <= 5) || op == 7)
            {
                printf("\n");
                num2 = LerDouble(" < Número > ");
                estado = 1;
                resposta = CalcularOperacaoC(op, num1, num2);
            }
            else if (op == 8)
            {
                Log_Sistema("Usuario saiu da Calculadora Cientifica");
                return;
            }
            else
            {
                LimpaTela();
                Log_Erro(MOD_CALC_CIENTIFICA, "OP_INV", "Operador invalido escolhido");
                Linha(39, 1, 0);
                printf(RED_B_BK " < ERRO > " RST BOLD RED_B "        ÍNDICE DE OPERADOR INVÁLIDO " RST "\n");
                fflush(stdout);
                Linha(39, 1, 0);
            }
        }
        else if (estado == 1)
        {
            LimpaTela();
            Cabecalho("Calculadora - Científica", 0);
            printf("\n" BLK_B_BK " < Operações Permitidas > " RST "\n");
            Linha(39, 1, 0);
            for (int i = 0; i < 8; ++i)
                printf(" %s " RST BOLD "  >  " RST "\n", OP[i]);
            Linha(39, 1, 0);
            Resposta((float)resposta);
            printf("\n\n");
            op = LerInt("    < Operador >   ");
            if (op < 1 || op > 8)
            {
                LimpaTela();
                Log_Erro(MOD_CALC_CIENTIFICA, "OP_INV", "Operador invalido no estado 1");
                Linha(39, 1, 0);
                printf("ÍNDICE DE OPERADOR INVÁLIDO\n");
                Linha(39, 1, 0);
                printf("\nNúmero Anterior [  %.0f  ]\n", resposta);
                continue;
            }
            if (op == 8)
            {
                Log_Sistema("Usuario saiu da Calculadora Cientifica");
                return;
            }
            if (op == 6)
            {
                resposta = RaizC(resposta);
                Log_Operacao(MOD_CALC_CIENTIFICA, "RAIZ", "Raiz encadeada");
                continue;
            }
            num2 = LerDouble("      < Número >   ");
            resposta = CalcularOperacaoC(op, resposta, num2);
        }
    }
}

//gera a sequência de forma iterativa (com 3 variáveis andando), não recursiva
//t1 = termo atual, t2 = próximo termo, t3 = guarda a soma antes de "andar" as variáveis pra frente
//recursivo seria mais bonito de olhar mas empilha uma chamada de função por termo - iterativo é O(n) e não estoura pilha
void GerarFibonacci(float *vet, int tamanho)
{
    float t1 = 0.0f, t2 = 1.0f, t3 = 0.0f;
    for (int i = 0; i < tamanho; ++i)
    {
        vet[i] = t1;
        printf(BLK_B_BK " %.0f " RST, t1);
        if (i < tamanho - 1)
            printf(" - ");
        else
            printf(" >> \n" RST);
        t3 = t1 + t2;
        t1 = t2;
        t2 = t3;
    }
}

void Contador(int cont)
{
    for (int i = 1; i <= cont; i++)
    {
        printf(BLU_B_BK "\r < Contador Para Reinício > " BLK_B_BK "  %d  " RST, i);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

void Fibonacci(int n)
{
    if (n <= 0)
    {
        Log_Erro(MOD_CALC_UNI, "FIB_NEG", "Termos invalidos para Fibonacci");
        return;
    }
    //aloca em tempo de execução porque só sabemos "n" depois que o usuário digita
    //um vetor fixo (ex: float vet[100]) ia desperdiçar memória se n for pequeno e quebrar se for maior que 100
    float *vet = malloc((size_t)n * sizeof(float));
    if (!vet) //malloc pode falhar (memória cheia) - sempre checar antes de usar o ponteiro
    {
        Log_Erro(MOD_CALC_UNI, "MEM_ERR", "Erro de alocacao para Fibonacci");
        printf(RED_B_BK " Erro ao alocar memória! " RST "\n");
        return;
    }
    printf(BLU_B_BK " Sequencia > " BLK_B_BK ITA " FIBONACCI " RST);
    printf("\n\n");
    GerarFibonacci(vet, n);
    Log_Operacao(MOD_CALC_UNI, "FIBONACCI", "Sequencia de Fibonacci gerada");
    Linha(39, 2, 7);
    Contador(5);
    free(vet); //devolve a memória pro sistema - todo malloc precisa de um free, senão é memory leak
}

//só checa as regras matemáticas de uma PA (progressão aritmética) e devolve um código de erro (0 = tudo certo)
//separei essa checagem "pura" da parte interativa de baixo de propósito: essa aqui não imprime nada,
//não lê nada do teclado, só recebe números e devolve um código - fica fácil de testar sozinha
int VerificaPA_Logica(float a1, float a2, float razao)
{
    if (razao == 0.0f)
        return 1;
    if (a1 < a2 && razao < 0.0f)
        return 2;
    if (a1 > a2 && razao > 0.0f)
        return 3;
    if (a1 < a2 && (a1 + razao) > a2)
        return 4;
    if (a1 > a2 && (a1 + razao) < a2)
        return 5;
    return 0;
}

//recebe PONTEIRO pros 3 valores (float *a1 em vez de float a1) porque essa função precisa poder
//MUDAR o início/fim/razão que o usuário digitou lá em CalculadoraU e essa mudança tem que "voltar" pra lá
//se fosse por valor normal, qualquer alteração aqui dentro se perderia ao sair da função
bool VerificaPA_Interativo(float *a1, float *a2, float *razao)
{
    while (1)
    {
        int code = VerificaPA_Logica(*a1, *a2, *razao);
        if (code == 0)
            return true;

        char logDesc[100];
        snprintf(logDesc, 100, "Erro PA Codigo %d", code);
        Log_Erro(MOD_CALC_UNI, "PA_LOGICA_ERR", logDesc);

        if (code == 1)
            printf("\n\n" RED_B_BK " A razão não pode ser 0! " RST "\n\n");
        if (code == 2)
            printf("\n\n" RED_B_BK " A razão de uma PA Crescente deve ser Positiva! " RST "\n\n");
        if (code == 3)
            printf("\n\n" RED_B_BK " A razão de uma PA Decrescente deve ser Negativa! " RST "\n\n");
        if (code == 4 || code == 5)
            printf("\n\n" RED_B_BK " A razão é maior que a diferença total! " RST "\n\n");

        char esc = LerCharSN(BOLD CYN ITA "Deseja trocar algum dado para êxito? [S/N] " RST);
        if (esc == 'N')
            return false;
        char sel = LerCharOpcao(BOLD BLU ITA "Qual dado deseja mudar? [R=razao/I=inicio/F=fim]: " RST, "RIF");
        if (sel == 'R' || sel == 'r')
        {
            *razao = LerFloat(BOLD BLU_B_BK "Digite a nova Razão: " RST " ");
            continue;
        }
        if (sel == 'I' || sel == 'i')
        {
            *a1 = LerFloat(BOLD BLK_B_BK "Digite o novo Início:" RST " ");
            continue;
        }
        if (sel == 'F' || sel == 'f')
        {
            *a2 = LerFloat(BOLD MAG_B_BK "Digite o novo Fim:" RST " ");
            continue;
        }
    }
}

void ImprimirPA(float a1, float a2, float razao)
{
    LimpaTela();
    printf("\n" BLU_B_BK " Progressão > " BLK_B_BK ITA " ARITMÉTICA " RST);
    printf("\n\n");
    if (a1 == a2)
    {
        printf("\n" BLK_B_BK " %.0f " RST "\n", a1);
        return;
    }
    if (a1 < a2)
    {
        for (float i = a1; i <= a2; i += razao)
        {
            printf(BLK_B_BK " %.0f " RST, i);
            if (i < a2)
                printf(" - ");
            else
                printf(" > ");
        }
    }
    else
    {
        for (float i = a1; i >= a2; i += razao)
        {
            printf(BLK_B_BK " %.0f " RST, i);
            if (i < a2)
                printf(" - ");
            else
                printf(" >  -  FIM");
        }
    }
    printf("\n");
    Log_Operacao(MOD_CALC_UNI, "PA_PRINT", "Progressao Aritmetica impressa");
}

int ProgressaoArTm(float a1, float a2, float razao)
{
    while (1)
    {
        if (!VerificaPA_Interativo(&a1, &a2, &razao))
            return 1;
        ImprimirPA(a1, a2, razao);
        char cont = LerCharSN("\n" BOLD YEL_B "Deseja calcular outra PA? [S/N]:" RST " ");
        if (cont == 'N')
        {
            return 1;
        }
        else if (cont == 'S')
        {
            return 0;
        }
    }
}

void CalculadoraU(void)
{
    double num1 = 0.0, num2 = 0.0, resultado = 0.0;
    float Fb = 0.0f;
    int choice = 0;
    const char *OP[] = {
        BOLD "     <  " CYN_BK " -- " BLU_BK " Adição          " BOLD MAG_BK " [  +  ] " BOLD GRN_BK "    --ID = 1 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Subtração       " BOLD MAG_BK " [  -  ] " BOLD GRN_BK "    --ID = 2 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Multiplicação   " BOLD MAG_BK " [  x  ] " BOLD GRN_BK "    --ID = 3 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Divisão         " BOLD MAG_BK " [  ÷  ] " BOLD GRN_BK "    --ID = 4 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Fibonacci       " BOLD MAG_BK " [ 1-5 ] " BOLD GRN_BK "    --ID = 5 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Progressão      " BOLD MAG_BK " [ 2x5 ] " BOLD GRN_BK "    --ID = 6 ",
        BOLD "     <  " CYN_BK " -- " BLU_BK " Saída           " BOLD MAG_BK " [ ExT ] " BOLD GRN_BK "    --ID = 7 ",
    };

    while (1)
    {
        LimpaTela();
        Cabecalho("Calculadora - UNI", 0);
        printf("\n" BOLD BLK_B_BK " < Operações Permitidas > " RST "\n");
        Linha(39, 1, 0);

        for (int i = 0; i < 7; ++i)
        {
            printf(" %s " RST BOLD "  >  "
                   "\n" RST,
                   OP[i]);
            slep(0.02f);
            if (i == 6)
            {
                Linha(39, 1, 0);
            }
        }

        choice = LerInt(" < Escolha > ");
        printf("\n");
        Linha(39, 1, 0);
        if (choice == 7)
        {
            Log_Sistema("Usuario saiu da Calculadora UNI");
            break;
        }
        if (choice < 1 || choice > 7)
        {
            Log_Erro(MOD_CALC_UNI, "OP_INV", "Escolha invalida");
            printf("Dado Invalido\n");
            slep(1);
            continue;
        }

        switch (choice)
        {
        case 1:
            num1 = LerDouble("  < Número >   ");
            num2 = LerDouble("\n  < Número >   ");
            resultado = AdiU(num1, num2);
            Log_Operacao(MOD_CALC_UNI, "ADICAO", "Soma UNI realizada");
            LimpaTela();
            break;
        case 2:
            num1 = LerDouble("  < Número >   ");
            num2 = LerDouble("\n  < Número >   ");
            resultado = SubU(num1, num2);
            Log_Operacao(MOD_CALC_UNI, "SUBTRACAO", "Subtracao UNI realizada");
            LimpaTela();
            break;
        case 3:
            num1 = LerDouble("  < Número >  ");
            num2 = LerDouble("\n  < Número >  ");
            resultado = MultiU(num1, num2);
            Log_Operacao(MOD_CALC_UNI, "MULTIPLICACAO", "Multiplicacao UNI realizada");
            LimpaTela();
            break;
        case 4:
            num1 = LerDouble("  < Número >   ");
            num2 = LerDouble("\n  < Número >   ");
            resultado = DiviU(num1, num2);
            Log_Operacao(MOD_CALC_UNI, "DIVISAO", "Divisao UNI realizada");
            LimpaTela();
            break;
        case 5:
            Fb = (float)LerInt("  < Número de Termos > ");
            LimpaTela();
            Fibonacci((int)Fb);
            continue;
        case 6:
        {
            int fluxo = 0;
            while (fluxo == 0)
            {
                float a1 = LerFloat(BOLD MAG_B " Digite o início da PA > " RST " ");
                float a2 = LerFloat(BOLD BLU_B " Digite o Número final da PA > " RST " ");
                float a3 = LerFloat(BOLD MAG_B " Digite a razão da PA > " RST " ");
                fluxo = ProgressaoArTm(a1, a2, a3);
                LimpaTela();
                Cabecalho("Calculadora - UNI", 0);
            }
            continue;
        }
        }
        Resposta((float)resultado);
        printf("\nPressione qualquer tecla para continuar...");
        ClearInputUntilNewline();
    }
}

int main(void)
{
    Log_Init();
    Log_Sistema("--- SESSAO INICIADA ---");
    int choice = 0;
    while (1)
    {
        LimpaTela();
        Cabecalho("Calculadora - Dinâmica", 0);
        puts(BOLD CYN_B_BK "\n\n     < OPÇÕES DE CALCULADORA >     " RST);
        Linha(39, 1, 0);
        puts("");
        puts(BOLD MAG_B_BK " < 1 > " RST BOLD " - Calculadora Ciêntífica \n");
        puts(BOLD MAG_B_BK " < 2 > " RST BOLD " - Calculadora Uni-Funcional \n");
        puts(BOLD RED_B_BK " < 3 > " RST BOLD " - Encerrar Programa \n");
        Linha(39, 1, 0);
        choice = LerInt(" < ESCOLHA >  ");

        if (choice == 1)
        {
            Log_Sistema("Entrou Modo Cientifico");
            puts("\n" BOLD GRN "Entrando na calculadora ciêntífica..aguarde");
            slep(1.0f);
            puts("validando Cálculos e valores..." RST);
            slep(1.0f);
            LimpaTela();
            CalculadoraC();
        }
        else if (choice == 2)
        {
            Log_Sistema("Entrou Modo UNI");
            puts("\n" BOLD GRN "Entrando na calculadora - UNI..aguarde");
            slep(1.0f);
            puts("validando Cálculos e valores..." RST);
            slep(1.0f);
            LimpaTela();
            CalculadoraU();
        }
        else if (choice == 3)
        {
            Log_Sistema("Programa encerrado via menu");
            printf(BOLD RED "\nSaindo...\n" RST);
            slep(1.0f);
            break;
        }
        else
        {
            Log_Erro(MOD_SISTEMA, "MENU_ERR", "Escolha invalida no menu principal");
            LimpaTela();
            printf("\n\n< ESCOLHA INVÁLIDA TENTE NOVAMENTE >\n\n\n\n");
            slep(1.5f);
        }
    }
    return 0;
}