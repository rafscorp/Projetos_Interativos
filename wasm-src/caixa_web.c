#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

// ponte assincrona com o JS: pausa a execucao do wasm (via ASYNCIFY) ate o
// usuario digitar uma linha no terminal e apertar Enter. Isso substitui
// o scanf(), que so sabe ler de um stdin sincrono que nao existe de
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

void limparTela()
{
    web_clear_screen();
}

void ImprimeLine(int lin1)
{
    for (int i = 0; i < 40; i++)
    {
        if (lin1 == 1) printf(BOLD GREEN "--" RESET);
        else if (lin1 == 2) printf(BOLD CYAN "--" RESET);
    }
    printf("\n");
}

void ImprimeCabecalho(const char *titulo)
{
    limparTela();
    ImprimeLine(1);
    printf(BOLD CYAN "                    %s\n" RESET, titulo);
    ImprimeLine(1);
}

float lerFloatWeb(const char *mensagem)
{
    char buf[64];
    float valor;
    while (1)
    {
        printf("%s\n", mensagem);
        fflush(stdout);
        web_read_line(buf, sizeof(buf));
        if (sscanf(buf, "%f", &valor) == 1) return valor;
        printf(BOLD RED "Entrada invalida, tente de novo.\n" RESET);
    }
}

float DinheiroFloat()
{
    float totD;
    float totC = 0;
    float sac;
    float cMonetaria = 200;
    while (1)
    {
        ImprimeLine(2);
        totD = lerFloatWeb("Quanto Reais deseja ter?  " BOLD GREEN "R$" RESET " ");
        ImprimeLine(2);
        if (totD <= 0)
        {
            printf(BOLD RED "\nValor invalido! Tente novamente.\n" RESET);
            ImprimeLine(2);
        }
        else
        {
            while (1)
            {
                sac = lerFloatWeb("Quanto R$ voce deseja Sacar? (Digite 0 para sair)  " BOLD GREEN "R$" RESET " ");
                printf(BOLD GREEN "Sacando...\n" RESET);
                ImprimeLine(2);

                if (sac == 0)
                {
                    printf(BOLD YELLOW "Saindo do saque...\n" RESET);
                    return totD;
                }

                if (sac < 0 || sac > totD)
                {
                    printf(BOLD RED "\nValor invalido! Tente novamente.\n" RESET);
                }
                else
                {
                    totD -= sac;
                    while (1)
                    {
                        if (sac >= cMonetaria)
                        {
                            sac -= cMonetaria;
                            totC += 1;
                        }
                        else
                        {
                            if (totC > 0)
                            {
                                printf(BOLD YELLOW "Voce recebeu " RESET BOLD CYAN "%.0f" RESET BOLD YELLOW " cedula(s) de R$%.0f\n" RESET, totC, cMonetaria);
                                ImprimeLine(2);
                                totC = 0;
                            }
                            if (cMonetaria == 200) cMonetaria = 100;
                            else if (cMonetaria == 100) cMonetaria = 50;
                            else if (cMonetaria == 50) cMonetaria = 20;
                            else if (cMonetaria == 20) cMonetaria = 10;
                            else if (cMonetaria == 10) cMonetaria = 5;
                            else if (cMonetaria == 5) cMonetaria = 2;
                            else if (cMonetaria == 2) cMonetaria = 1;
                            if (sac == 0)
                            {
                                cMonetaria = 200;
                                printf(BOLD GREEN "\nSaque realizado com sucesso! Saldo restante: R$%.2f\n" RESET, totD);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    ImprimeCabecalho("Bem-vindo ao Caixa eletronico CYPRUS");
    printf(BOLD YELLOW "\n Entrando... - USUARIO - (" RESET BOLD CYAN " ADRIAN_RAFAEL " RESET BOLD YELLOW ")\n" RESET);
    emscripten_sleep(500);
    DinheiroFloat();
    return 0;
}
