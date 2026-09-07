import java.io.IOException;
import java.util.List;
import java.util.Scanner;

//essa classe só cuida do menu (mostrar opções, ler o que o usuário digitou, chamar o método
//certo do GerenciadorEstoque). Ela não sabe NADA sobre as regras de negócio - se soubesse, toda
//vez que uma regra mudasse a gente ia ter que mexer no menu também, e isso é exatamente o tipo
//de acoplamento que a gente quer evitar numa aplicação orientada a objetos
public class Main {
    private static final String ARQUIVO_PRODUTOS = "produtos.txt";
    private static final String ARQUIVO_MOVIMENTACOES = "movimentacoes.txt";

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        GerenciadorEstoque estoque = new GerenciadorEstoque();

        carregarDados(estoque);

        boolean continuar = true;
        while (continuar) {
            exibirMenu();
            int opcao = lerInteiro(scanner, "Escolha uma opcao: ");

            switch (opcao) {
                case 1:
                    cadastrarProduto(scanner, estoque);
                    break;
                case 2:
                    listarProdutos(estoque);
                    break;
                case 3:
                    registrarEntrada(scanner, estoque);
                    break;
                case 4:
                    registrarSaida(scanner, estoque);
                    break;
                case 5:
                    listarHistorico(estoque);
                    break;
                case 6:
                    new RelatorioEstoqueBaixo().gerar(estoque);
                    break;
                case 7:
                    new RelatorioMaisMovimentados().gerar(estoque);
                    break;
                case 0:
                    salvarDados(estoque);
                    System.out.println("Dados salvos. Ate a proxima!");
                    continuar = false;
                    break;
                default:
                    System.out.println("Opcao invalida, tenta de novo.");
            }
        }

        scanner.close();
    }

    private static void exibirMenu() {
        System.out.println();
        System.out.println("========= SISTEMA DE ESTOQUE =========");
        System.out.println("1 - Cadastrar produto");
        System.out.println("2 - Listar produtos");
        System.out.println("3 - Registrar entrada de estoque");
        System.out.println("4 - Registrar saida de estoque");
        System.out.println("5 - Ver historico de movimentacoes");
        System.out.println("6 - Relatorio: estoque baixo");
        System.out.println("7 - Relatorio: produtos mais movimentados");
        System.out.println("0 - Salvar e sair");
        System.out.println("=======================================");
    }

    private static void cadastrarProduto(Scanner scanner, GerenciadorEstoque estoque) {
        try {
            String nome = lerTexto(scanner, "Nome do produto: ");
            String categoria = lerTexto(scanner, "Categoria: ");
            int quantidade = lerInteiro(scanner, "Quantidade inicial: ");
            int estoqueMinimo = lerInteiro(scanner, "Estoque minimo (alerta): ");
            double preco = lerDouble(scanner, "Preco unitario (ex: 19.90): ");

            Produto produto = estoque.cadastrarProduto(nome, categoria, quantidade, estoqueMinimo, preco);
            System.out.println("Produto cadastrado com sucesso: " + produto);
        } catch (EstoqueException e) {
            System.out.println("Erro ao cadastrar: " + e.getMessage());
        }
    }

    private static void listarProdutos(GerenciadorEstoque estoque) {
        List<Produto> produtos = estoque.listarProdutos();
        if (produtos.isEmpty()) {
            System.out.println("Nenhum produto cadastrado ainda.");
            return;
        }

        System.out.println();
        for (Produto produto : produtos) {
            System.out.println(produto);
        }
    }

    private static void registrarEntrada(Scanner scanner, GerenciadorEstoque estoque) {
        try {
            int id = lerInteiro(scanner, "ID do produto: ");
            int quantidade = lerInteiro(scanner, "Quantidade de entrada: ");
            String observacao = lerTexto(scanner, "Observacao (motivo da entrada): ");

            estoque.registrarEntrada(id, quantidade, observacao);
            System.out.println("Entrada registrada com sucesso.");
        } catch (EstoqueException e) {
            System.out.println("Erro ao registrar entrada: " + e.getMessage());
        }
    }

    private static void registrarSaida(Scanner scanner, GerenciadorEstoque estoque) {
        try {
            int id = lerInteiro(scanner, "ID do produto: ");
            int quantidade = lerInteiro(scanner, "Quantidade de saida: ");
            String observacao = lerTexto(scanner, "Observacao (motivo da saida): ");

            estoque.registrarSaida(id, quantidade, observacao);
            System.out.println("Saida registrada com sucesso.");

            Produto produto = estoque.buscarProduto(id);
            if (produto.estoqueBaixo()) {
                System.out.println("Atencao: " + produto.getNome() + " ficou com estoque baixo ("
                        + produto.getQuantidade() + " unidade(s)).");
            }
        } catch (EstoqueException e) {
            System.out.println("Erro ao registrar saida: " + e.getMessage());
        }
    }

    private static void listarHistorico(GerenciadorEstoque estoque) {
        List<Movimentacao> movimentacoes = estoque.getMovimentacoes();
        if (movimentacoes.isEmpty()) {
            System.out.println("Nenhuma movimentacao registrada ainda.");
            return;
        }

        System.out.println();
        for (Movimentacao mov : movimentacoes) {
            System.out.println(mov);
        }
    }

    private static void carregarDados(GerenciadorEstoque estoque) {
        try {
            estoque.carregar(ARQUIVO_PRODUTOS, ARQUIVO_MOVIMENTACOES);
            System.out.println("Dados carregados de " + ARQUIVO_PRODUTOS + " e " + ARQUIVO_MOVIMENTACOES + ".");
        } catch (IOException e) {
            System.out.println("Nao foi possivel carregar dados salvos (comecando do zero): " + e.getMessage());
        }
    }

    private static void salvarDados(GerenciadorEstoque estoque) {
        try {
            estoque.salvar(ARQUIVO_PRODUTOS, ARQUIVO_MOVIMENTACOES);
        } catch (IOException e) {
            System.out.println("Erro ao salvar dados: " + e.getMessage());
        }
    }

    //le uma linha inteira e tenta converter pra inteiro; se a pessoa digitar besteira o programa
    //não quebra (nao uso scanner.nextInt() de proposito, porque ele nao consome o Enter e trava
    //o proximo nextLine() logo depois - classico bug de quem tá comecando com Scanner)
    private static int lerInteiro(Scanner scanner, String mensagem) {
        while (true) {
            System.out.print(mensagem);
            String entrada = scanner.nextLine().trim();
            try {
                return Integer.parseInt(entrada);
            } catch (NumberFormatException e) {
                System.out.println("Digite um numero inteiro valido.");
            }
        }
    }

    private static double lerDouble(Scanner scanner, String mensagem) {
        while (true) {
            System.out.print(mensagem);
            String entrada = scanner.nextLine().trim().replace(",", ".");
            try {
                return Double.parseDouble(entrada);
            } catch (NumberFormatException e) {
                System.out.println("Digite um valor numerico valido.");
            }
        }
    }

    private static String lerTexto(Scanner scanner, String mensagem) {
        System.out.print(mensagem);
        return scanner.nextLine().trim();
    }
}
