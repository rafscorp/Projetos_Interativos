import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

//cada movimentação é um registro histórico: depois de criada, ninguém edita nem apaga, só
//guarda "o que aconteceu" (produto, tipo, quantidade, quando, motivo) - é basicamente um log,
//igual um extrato bancário que só cresce, nunca reescreve o passado
public class Movimentacao implements Persistivel {
    private static final DateTimeFormatter FORMATO_DATA = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    private final int produtoId;
    private final TipoMovimentacao tipo;
    private final int quantidade;
    private final LocalDateTime dataHora;
    private final String observacao;

    public Movimentacao(int produtoId, TipoMovimentacao tipo, int quantidade, LocalDateTime dataHora, String observacao) {
        this.produtoId = produtoId;
        this.tipo = tipo;
        this.quantidade = quantidade;
        this.dataHora = dataHora;
        this.observacao = observacao;
    }

    public int getProdutoId() {
        return produtoId;
    }

    public TipoMovimentacao getTipo() {
        return tipo;
    }

    public int getQuantidade() {
        return quantidade;
    }

    public LocalDateTime getDataHora() {
        return dataHora;
    }

    public String getObservacao() {
        return observacao;
    }

    @Override
    public String paraLinha() {
        String obsSegura = observacao.replace(";", ",");
        return produtoId + ";" + tipo.name() + ";" + quantidade + ";" + dataHora.format(FORMATO_DATA) + ";" + obsSegura;
    }

    public static Movimentacao fromLinha(String linha) {
        String[] partes = linha.split(";", -1);
        int produtoId = Integer.parseInt(partes[0]);
        TipoMovimentacao tipo = TipoMovimentacao.valueOf(partes[1]);
        int quantidade = Integer.parseInt(partes[2]);
        LocalDateTime dataHora = LocalDateTime.parse(partes[3], FORMATO_DATA);
        String observacao = partes[4];
        return new Movimentacao(produtoId, tipo, quantidade, dataHora, observacao);
    }

    @Override
    public String toString() {
        String sinal = tipo == TipoMovimentacao.ENTRADA ? "+" : "-";
        return String.format("[%s] produto #%d %s%d  (%s)",
                dataHora.format(FORMATO_DATA), produtoId, sinal, quantidade, observacao);
    }
}
