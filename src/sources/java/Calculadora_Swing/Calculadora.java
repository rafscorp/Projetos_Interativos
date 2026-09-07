import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

//calculadora clássica em Swing: uma janela (JFrame) com um campo de texto em cima (o "visor")
//e uma grade de botões embaixo. Cada botão dispara o mesmo actionPerformed, e a gente decide o
//que fazer olhando o texto do botão que foi clicado (getActionCommand()) - assim não precisa de
//uma classe anônima diferente pra cada botão, só uma classe implementando ActionListener pra todos
public class Calculadora extends JFrame implements ActionListener {

    private final JTextField visor;

    //guarda o valor que já foi "fechado" antes do operador atual (ex: em "5 + 3", depois de
    //apertar o "+", valorAcumulado vira 5 e o visor fica livre pro usuário digitar o 3)
    private double valorAcumulado = 0;
    private String operadorPendente = null;
    private boolean comecarNumeroNovo = true;

    public Calculadora() {
        super("Calculadora");

        visor = new JTextField("0");
        visor.setEditable(false);
        visor.setHorizontalAlignment(JTextField.RIGHT);
        visor.setFont(new Font("Monospaced", Font.BOLD, 28));

        String[] botoes = {
                "7", "8", "9", "/",
                "4", "5", "6", "*",
                "1", "2", "3", "-",
                "0", ".", "=", "+"
        };

        JPanel painelBotoes = new JPanel(new GridLayout(4, 4, 4, 4));
        for (String rotulo : botoes) {
            JButton botao = new JButton(rotulo);
            botao.setFont(new Font("SansSerif", Font.PLAIN, 20));
            botao.addActionListener(this);
            painelBotoes.add(botao);
        }

        JButton botaoLimpar = new JButton("C");
        botaoLimpar.setFont(new Font("SansSerif", Font.BOLD, 18));
        botaoLimpar.addActionListener(this);

        setLayout(new BorderLayout(4, 4));
        add(visor, BorderLayout.NORTH);
        add(painelBotoes, BorderLayout.CENTER);
        add(botaoLimpar, BorderLayout.SOUTH);

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(320, 420);
        setLocationRelativeTo(null);
        setResizable(false);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        String comando = e.getActionCommand();

        if (comando.matches("[0-9]")) {
            digitar(comando);
        } else if (comando.equals(".")) {
            digitarPonto();
        } else if (comando.equals("C")) {
            limpar();
        } else if (comando.equals("=")) {
            calcular();
            operadorPendente = null;
        } else {
            definirOperador(comando);
        }
    }

    private void digitar(String digito) {
        if (comecarNumeroNovo) {
            visor.setText(digito);
            comecarNumeroNovo = false;
        } else if (visor.getText().equals("0")) {
            visor.setText(digito);
        } else {
            visor.setText(visor.getText() + digito);
        }
    }

    private void digitarPonto() {
        if (comecarNumeroNovo) {
            visor.setText("0.");
            comecarNumeroNovo = false;
        } else if (!visor.getText().contains(".")) {
            visor.setText(visor.getText() + ".");
        }
    }

    //quando aperta um operador com uma conta pendente, a calculadora resolve a conta anterior
    //primeiro (tipo calculadora de bolso mesmo: "5 + 3 + 2" vai somando de dois em dois, sem
    //respeitar precedência matemática - isso é proposital, é o comportamento clássico desse tipo de UI)
    private void definirOperador(String operador) {
        if (operadorPendente != null && !comecarNumeroNovo) {
            calcular();
        } else {
            valorAcumulado = Double.parseDouble(visor.getText());
        }
        operadorPendente = operador;
        comecarNumeroNovo = true;
    }

    private void calcular() {
        if (operadorPendente == null) {
            return;
        }

        double valorAtual = Double.parseDouble(visor.getText());
        double resultado;

        switch (operadorPendente) {
            case "+":
                resultado = valorAcumulado + valorAtual;
                break;
            case "-":
                resultado = valorAcumulado - valorAtual;
                break;
            case "*":
                resultado = valorAcumulado * valorAtual;
                break;
            case "/":
                //divisão por zero não pode virar Infinity silencioso no visor - melhor avisar
                //e resetar a calculadora do que deixar o usuário achando que o resultado é válido
                if (valorAtual == 0) {
                    visor.setText("Erro: div. por 0");
                    valorAcumulado = 0;
                    operadorPendente = null;
                    comecarNumeroNovo = true;
                    return;
                }
                resultado = valorAcumulado / valorAtual;
                break;
            default:
                resultado = valorAtual;
        }

        valorAcumulado = resultado;
        visor.setText(formatarResultado(resultado));
        comecarNumeroNovo = true;
    }

    //sem isso, uma conta tipo "4 + 2" ia mostrar "6.0" no visor em vez de "6" - só cai pro
    //formato com casa decimal quando o resultado realmente não é um número inteiro
    private String formatarResultado(double valor) {
        if (!Double.isInfinite(valor) && !Double.isNaN(valor) && valor == Math.rint(valor)
                && Math.abs(valor) < 1_000_000_000L) {
            return String.valueOf((long) valor);
        }
        return String.valueOf(valor);
    }

    private void limpar() {
        visor.setText("0");
        valorAcumulado = 0;
        operadorPendente = null;
        comecarNumeroNovo = true;
    }

    public static void main(String[] args) {
        //SwingUtilities.invokeLater garante que a interface é criada na Event Dispatch Thread -
        //a thread especial do Swing que cuida de desenhar tudo. Criar componentes fora dela
        //funciona na maioria das vezes, mas pode dar bug esquisito e intermitente depois
        SwingUtilities.invokeLater(() -> {
            Calculadora calculadora = new Calculadora();
            calculadora.setVisible(true);
        });
    }
}
