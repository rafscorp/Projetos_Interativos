import java.util.List;

public class RelatorioEstoqueBaixo extends Relatorio {
    @Override
    public void gerar(GerenciadorEstoque estoque) {
        titulo("Produtos com estoque baixo");

        List<Produto> baixos = estoque.listarEstoqueBaixo();
        if (baixos.isEmpty()) {
            System.out.println("Nenhum produto abaixo do estoque minimo. Tudo certo por aqui.");
            return;
        }

        for (Produto produto : baixos) {
            System.out.println(produto);
        }
    }
}
