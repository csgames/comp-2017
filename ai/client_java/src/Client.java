import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class Client {
    private final Random randomGenerator;
    protected String host;
    protected int port;
    protected List<String> actions = new ArrayList<String>();
    ;
    protected String name = "Rick";

    public Client(String host, int port) {
        this.host = host;
        this.port = port;
        initialize_actions();
        randomGenerator = new Random();
    }

    private void initialize_actions() {
        actions.add("north west");
        actions.add("north");
        actions.add("north east");
        actions.add("east");
        actions.add("south east");
        actions.add("south");
        actions.add("south west");
        actions.add("west");
    }

    public static void main(String[] args) {
        Client c = new Client("localhost", 8023);
        try {
            c.play_game();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void play_game() throws IOException {

        Socket pingSocket = null;
        PrintWriter out = null;
        BufferedReader in = null;

        pingSocket = new Socket(this.host, this.port);
        out = new PrintWriter(pingSocket.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(pingSocket.getInputStream()));
        String server_response = "";
        while (true) {
            server_response = in.readLine();
            System.out.println("Server said:" + server_response);
            if (server_response.equals("What's your name?")) {
                out.println(name + "\r");
            } else {
                if (server_response.startsWith(name + " is active player") || server_response.startsWith("invalid move")) {
                    String msg = actions.get(randomGenerator.nextInt(actions.size()));
                    System.out.println(msg);
                    out.println(msg+"\r");
                } else if (server_response.startsWith("Game is on")) {
                    System.out.println("Game ON");
                } else if (server_response.contains("won a goal was made") || server_response.contains("checkmate")) {
                    System.out.println("Game is done");
                    break;
                }
            }
        }
        out.close();
        in.close();
        pingSocket.close();
    }
}
