//interface simples: só define um jeito comum de virar linha de texto pra salvar em arquivo.
//tanto Produto quanto Movimentacao implementam isso, aí o GerenciadorEstoque consegue salvar
//os dois com o mesmo método (salvarEmArquivo), sem precisar duplicar código de escrita de arquivo
//pra cada tipo de dado diferente
public interface Persistivel {
    String paraLinha();
}
