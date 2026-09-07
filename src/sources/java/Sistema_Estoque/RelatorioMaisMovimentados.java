import java.util.List;
import java.util.Map;

public class RelatorioMaisMovimentados extends Relatorio {
    @Override
    public void gerar(GerenciadorEstoque estoque) {
        titulo("Produtos mais movimentados");

        List<Map.Entry<Produto, Integer>> ranking = estoque.relatorioMaisMovimentados();
        if (ranking.isEmpty()) {
            System.out.println("Ainda nao ha movimentacoes registradas.");
            return;
        }

        int posicao = 1;
        for (Map.Entry<Produto, Integer> entrada : ranking) {
            System.out.printf("%d. %s - %d unidade(s) movimentadas no total%n",
                    posicao, entrada.getKey().getNome(), entrada.getValue());
            posicao++;
        }
    }
}
