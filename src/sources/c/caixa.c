#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

//Parte das definições e libs
void limparTela()
{
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
}
//Função de limpar a tela
void ImprimeLine(int lin1)
{
    for (int i = 0; i < 40; i++)
    {
        if (lin1 == 1)
        {
            printf(BOLD GREEN "--" RESET);
        }
        else if (lin1 == 2)
        {
            printf(BOLD CYAN "--" RESET);
        }
    }
    printf("\n");
}
//imprime uma linha
void ImprimeCabecalho(const char *titulo)
{
    limparTela();
    ImprimeLine(1);
    printf(BOLD CYAN "                    %s\n" RESET, titulo);
    ImprimeLine(1);
}
//Imprime o Titulo
float DinheiroFloat()
{
    float totD;
    float totC = 0;          //variavies
    float sac;
    float cMonetaria = 200;
    while (1)
    {
        ImprimeLine(2);
        printf("Quanto Reais deseja ter?  " BOLD GREEN "R$" RESET);
        scanf("%f", &totD);
        ImprimeLine(2);
        if (totD <= 0)
        {
            printf(BOLD RED "\nValor inválido! Tente novamente.\n" RESET);
            ImprimeLine(2);
        }
        else
        {
            while (1)
            {
                printf("Quanto R$ você deseja Sacar? (Digite 0 para sair)  " BOLD GREEN "R$" RESET);
                scanf("%f", &sac);
                printf(BOLD GREEN "Sacando...\n" RESET);
                limparTela();
                ImprimeLine(2);

                if (sac == 0)
                {
                    printf(BOLD YELLOW "Saindo do saque...\n" RESET);
                    return totD;
                }

                if (sac < 0 || sac > totD)
                {
                    printf(BOLD RED "\nValor inválido! Tente novamente.\n" RESET);
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
                            //fechou o que dava pra tirar dessa cédula, imprime quantas saíram e desce pra próxima
                            if (totC > 0)
                            {
                                printf(BOLD YELLOW "Você recebeu " RESET BOLD CYAN "%.0f" RESET BOLD YELLOW " cédula(s) de R$%.0f\n" RESET, totC, cMonetaria);
                                ImprimeLine(2);
                                totC = 0;
                            }
                            if (cMonetaria == 200)
                            {
                                cMonetaria = 100;
                            }
                            else if (cMonetaria == 100)
                            {
                                cMonetaria = 50;
                            }
                            else if (cMonetaria == 50)
                            {
                                cMonetaria = 20;
                            }
                            else if (cMonetaria == 20)
                            {
                                cMonetaria = 10;
                            }
                            else if (cMonetaria == 10)
                            {
                                cMonetaria = 5;
                            }
                            else if (cMonetaria == 5)
                            {
                                cMonetaria = 2;
                            }
                            else if (cMonetaria == 2)
                            {
                                cMonetaria = 1;
                            }
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
//essa função é o coração do programa: pergunta o saldo, depois fica em loop pedindo saques
//a distribuição de cédula é um algoritmo guloso (greedy): sempre tenta usar a maior cédula (200) primeiro
//e só desce pra próxima (100, 50, 20...) quando ela já não cabe mais no valor que falta sacar
//obs: usar float pra dinheiro funciona pra esse projeto, mas em sistema de verdade o certo é trabalhar
//com centavos em int, porque float perde precisão em casas decimais depois de várias contas

int main()   //codigo principal
{
    ImprimeCabecalho("Bem-vindo ao Caixa eletrônico CYPRUS");
    printf(BOLD YELLOW "\n Entrando... - USUÁRIO - (" RESET BOLD CYAN " ADRIAN_RAFAEL " RESET BOLD YELLOW ")\n" RESET); //nome fixo só de exemplo, não é login de verdade
    sleep(1);
    DinheiroFloat();
    return 0;
}