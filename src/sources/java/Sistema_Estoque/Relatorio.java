//classe abstrata pros relatórios: define a "forma" que todo relatório tem (um método gerar())
//mas quem implementa de verdade são as subclasses - cada relatório olha pro estoque de um jeito
//diferente (uma filtra por estoque baixo, outra soma movimentações e ordena), mas o Main só
//precisa saber chamar gerar() sem se importar com o que acontece por dentro de cada um
public abstract class Relatorio {
    public abstract void gerar(GerenciadorEstoque estoque);

    //método utilitário compartilhado pelas subclasses, pra não repetir a mesma linha de título
    //toda vez que um relatório novo for escrito
    protected void titulo(String texto) {
        System.out.println();
        System.out.println("=== " + texto + " ===");
    }
}
