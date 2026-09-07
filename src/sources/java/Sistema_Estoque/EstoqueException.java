//exceção própria pra regra de negócio (estoque insuficiente, produto que não existe, etc).
//assim quem usa o GerenciadorEstoque sabe exatamente o motivo do erro em vez de receber um
//RuntimeException genérico ou, pior, o programa quebrando sem explicação nenhuma
public class EstoqueException extends Exception {
    public EstoqueException(String mensagem) {
        super(mensagem);
    }
}
