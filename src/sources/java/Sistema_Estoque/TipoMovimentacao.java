//enum é melhor que usar String "entrada"/"saida" solta por aí: o compilador garante que só
//existem esses dois valores possíveis, sem risco de escrever "Entrada" com E maiúsculo num lugar
//e "entrada" minúsculo em outro e o sistema não bater as strings na hora de comparar
public enum TipoMovimentacao {
    ENTRADA,
    SAIDA
}
