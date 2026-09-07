import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

//essa classe é o "banco de dados" do sistema em memória: guarda os produtos num Map (busca por
//id é instantânea, sem precisar percorrer array) e as movimentações numa lista (ordem de chegada
//importa pro histórico). Toda regra de negócio (não deixar estoque negativo, etc) mora aqui,
//nunca dentro do Main - assim o menu só chama métodos prontos e nunca decide nada sozinho
public class GerenciadorEstoque {
    private final Map<Integer, Produto> produtos = new LinkedHashMap<>();
    private final List<Movimentacao> movimentacoes = new ArrayList<>();
    private int proximoId = 1;

    public Produto cadastrarProduto(String nome, String categoria, int quantidadeInicial, int estoqueMinimo, double precoUnitario) throws EstoqueException {
        if (nome == null || nome.trim().isEmpty()) {
            throw new EstoqueException("Nome do produto nao pode ser vazio.");
        }
        if (quantidadeInicial < 0 || estoqueMinimo < 0 || precoUnitario < 0) {
            throw new EstoqueException("Quantidade, estoque minimo e preco nao podem ser negativos.");
        }

        Produto produto = new Produto(proximoId, nome.trim(), categoria.trim(), quantidadeInicial, estoqueMinimo, precoUnitario);
        produtos.put(produto.getId(), produto);
        proximoId++;

        //cadastro com estoque inicial > 0 já entra como uma entrada no histórico, pra nenhum
        //produto "aparecer do nada" nos relatórios de movimentação mais tarde
        if (quantidadeInicial > 0) {
            movimentacoes.add(new Movimentacao(produto.getId(), TipoMovimentacao.ENTRADA, quantidadeInicial,
                    LocalDateTime.now(), "Estoque inicial do cadastro"));
        }

        return produto;
    }

    public Produto buscarProduto(int id) throws EstoqueException {
        Produto produto = produtos.get(id);
        if (produto == null) {
            throw new EstoqueException("Produto #" + id + " nao encontrado.");
        }
        return produto;
    }

    public void registrarEntrada(int produtoId, int quantidade, String observacao) throws EstoqueException {
        if (quantidade <= 0) {
            throw new EstoqueException("Quantidade de entrada precisa ser maior que zero.");
        }
        Produto produto = buscarProduto(produtoId);
        produto.ajustarQuantidade(quantidade);
        movimentacoes.add(new Movimentacao(produtoId, TipoMovimentacao.ENTRADA, quantidade, LocalDateTime.now(), observacao));
    }

    //essa é a regra mais importante do sistema: nunca deixar sair mais do que existe. Por isso
    //a validação vem ANTES de mexer na quantidade - se a exceção fosse lançada depois de já ter
    //descontado o estoque, o produto ficaria inconsistente mesmo com a operação "cancelada"
    public void registrarSaida(int produtoId, int quantidade, String observacao) throws EstoqueException {
        if (quantidade <= 0) {
            throw new EstoqueException("Quantidade de saida precisa ser maior que zero.");
        }
        Produto produto = buscarProduto(produtoId);
        if (produto.getQuantidade() < quantidade) {
            throw new EstoqueException("Estoque insuficiente: " + produto.getNome() + " tem apenas "
                    + produto.getQuantidade() + " unidade(s).");
        }
        produto.ajustarQuantidade(-quantidade);
        movimentacoes.add(new Movimentacao(produtoId, TipoMovimentacao.SAIDA, quantidade, LocalDateTime.now(), observacao));
    }

    public List<Produto> listarProdutos() {
        return new ArrayList<>(produtos.values());
    }

    public List<Produto> listarEstoqueBaixo() {
        List<Produto> resultado = new ArrayList<>();
        for (Produto produto : produtos.values()) {
            if (produto.estoqueBaixo()) {
                resultado.add(produto);
            }
        }
        return resultado;
    }

    public List<Movimentacao> getMovimentacoes() {
        return movimentacoes;
    }

    //soma quanto cada produto já movimentou (entrada + saida) e devolve ordenado do mais
    //movimentado pro menos - usa um HashMap só pra acumular o total antes de ordenar a lista final
    public List<Map.Entry<Produto, Integer>> relatorioMaisMovimentados() {
        Map<Integer, Integer> totalPorProduto = new HashMap<>();

        for (Movimentacao mov : movimentacoes) {
            totalPorProduto.merge(mov.getProdutoId(), mov.getQuantidade(), Integer::sum);
        }

        List<Map.Entry<Produto, Integer>> lista = new ArrayList<>();
        for (Map.Entry<Integer, Integer> entrada : totalPorProduto.entrySet()) {
            Produto produto = produtos.get(entrada.getKey());
            if (produto != null) {
                lista.add(new AbstractMap.SimpleEntry<>(produto, entrada.getValue()));
            }
        }

        lista.sort(Comparator.comparingInt((Map.Entry<Produto, Integer> e) -> e.getValue()).reversed());
        return lista;
    }

    public void salvar(String arquivoProdutos, String arquivoMovimentacoes) throws IOException {
        salvarEmArquivo(arquivoProdutos, new ArrayList<Persistivel>(produtos.values()));
        salvarEmArquivo(arquivoMovimentacoes, new ArrayList<Persistivel>(movimentacoes));
    }

    private void salvarEmArquivo(String caminho, List<Persistivel> itens) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(caminho))) {
            for (Persistivel item : itens) {
                writer.write(item.paraLinha());
                writer.newLine();
            }
        }
    }

    public void carregar(String arquivoProdutos, String arquivoMovimentacoes) throws IOException {
        produtos.clear();
        movimentacoes.clear();
        proximoId = 1;

        if (Files.exists(Paths.get(arquivoProdutos))) {
            try (BufferedReader reader = new BufferedReader(new FileReader(arquivoProdutos))) {
                String linha;
                while ((linha = reader.readLine()) != null) {
                    if (linha.trim().isEmpty()) continue;
                    Produto produto = Produto.fromLinha(linha);
                    produtos.put(produto.getId(), produto);
                    if (produto.getId() >= proximoId) {
                        proximoId = produto.getId() + 1;
                    }
                }
            }
        }

        if (Files.exists(Paths.get(arquivoMovimentacoes))) {
            try (BufferedReader reader = new BufferedReader(new FileReader(arquivoMovimentacoes))) {
                String linha;
                while ((linha = reader.readLine()) != null) {
                    if (linha.trim().isEmpty()) continue;
                    movimentacoes.add(Movimentacao.fromLinha(linha));
                }
            }
        }
    }
}
