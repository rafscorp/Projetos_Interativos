import java.util.Locale;

//classe que representa um produto do estoque - guarda todos os dados juntos (id, nome,
//categoria, quantidade...) numa coisa só, em vez de ter vários arrays soltos (um de nomes, um de
//quantidades...) onde o índice de um poderia desalinhar do índice do outro por engano
public class Produto implements Persistivel {
    private final int id;
    private String nome;
    private String categoria;
    private int quantidade;
    private int estoqueMinimo;
    private double precoUnitario;

    public Produto(int id, String nome, String categoria, int quantidade, int estoqueMinimo, double precoUnitario) {
        this.id = id;
        this.nome = nome;
        this.categoria = categoria;
        this.quantidade = quantidade;
        this.estoqueMinimo = estoqueMinimo;
        this.precoUnitario = precoUnitario;
    }

    public int getId() {
        return id;
    }

    public String getNome() {
        return nome;
    }

    public void setNome(String nome) {
        this.nome = nome;
    }

    public String getCategoria() {
        return categoria;
    }

    public void setCategoria(String categoria) {
        this.categoria = categoria;
    }

    public int getQuantidade() {
        return quantidade;
    }

    public int getEstoqueMinimo() {
        return estoqueMinimo;
    }

    public void setEstoqueMinimo(int estoqueMinimo) {
        this.estoqueMinimo = estoqueMinimo;
    }

    public double getPrecoUnitario() {
        return precoUnitario;
    }

    public void setPrecoUnitario(double precoUnitario) {
        this.precoUnitario = precoUnitario;
    }

    //sem modificador (pacote-privado) de propósito: só quem tá dentro do mesmo pacote (aqui,
    //só o GerenciadorEstoque) deveria poder mudar a quantidade direto, pra garantir que toda
    //alteração passa pelas validações dele em vez de mexer no número por fora escondido
    void ajustarQuantidade(int delta) {
        this.quantidade += delta;
    }

    public boolean estoqueBaixo() {
        return quantidade <= estoqueMinimo;
    }

    @Override
    public String paraLinha() {
        //troca ';' por ',' nos campos de texto livre pra não bagunçar o separador do arquivo -
        //é um cuidado simples, mas evita que um nome de produto com ';' quebre a leitura depois
        String nomeSeguro = nome.replace(";", ",");
        String categoriaSeguro = categoria.replace(";", ",");
        return id + ";" + nomeSeguro + ";" + categoriaSeguro + ";" + quantidade + ";" + estoqueMinimo + ";"
                + String.format(Locale.US, "%.2f", precoUnitario);
    }

    //monta um Produto de volta a partir da linha salva no arquivo - o formato tem que ser
    //exatamente o mesmo de paraLinha(), por isso os dois métodos ficam juntos na mesma classe
    public static Produto fromLinha(String linha) {
        String[] partes = linha.split(";", -1);
        int id = Integer.parseInt(partes[0]);
        String nome = partes[1];
        String categoria = partes[2];
        int quantidade = Integer.parseInt(partes[3]);
        int estoqueMinimo = Integer.parseInt(partes[4]);
        double precoUnitario = Double.parseDouble(partes[5]);
        return new Produto(id, nome, categoria, quantidade, estoqueMinimo, precoUnitario);
    }

    @Override
    public String toString() {
        String alerta = estoqueBaixo() ? "  [ESTOQUE BAIXO]" : "";
        return String.format(Locale.US, "#%d %-20s %-15s qtd=%-5d min=%-5d R$ %.2f%s",
                id, nome, categoria, quantidade, estoqueMinimo, precoUnitario, alerta);
    }
}
