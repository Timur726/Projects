import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;

public class Chess
{
  static private JFrame frame;

  public static void main(String args[])
  {
    frame = new JFrame("Chess");
    ChessJPanel cjp = new ChessJPanel();
    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    frame.getContentPane().setPreferredSize(new Dimension(cjp.getImageWidth(), cjp.getImageHeight()));
    frame.pack();
    frame.setLocationRelativeTo(null);
    frame.add(cjp);
    frame.setResizable(false);
    frame.setVisible(true);
  }

  public static int raiseCheckmate(int player)
  {
    return dialogMessages.displayCheckmate(frame, player*-1, false);
  }

  public static int raiseStalemate(int player)
  {
    return dialogMessages.displayCheckmate(frame, player*-1, true);
  }

  public static int raisePromotion(int player)
  {
    // 0 = queen, 1 = rook, 2 = bishop, 3 = knight
    int r = dialogMessages.displayPromotion(frame, player);

    // check if player exited by clicking the x
    if (r == -1) r = 5;

    if (r == 0) r = 5;
    if (r == 2) r = 4;
    if (r == 1) r = 2;

    return r*player;
  }

  public static void disposeJFrame()
  {
    frame.dispose();
  }
}

class ChessJPanel extends JPanel
{
  private Board b;
  private int[][] prevb;
  protected ArrayList<Integer> enpassantEligible;
  protected ArrayList<Integer> enpassantEligibleRow;
  private ImageIcon boardimage;
  private JButton button;
  private int player;
  private boolean playerIsMoving, moveSuccess, blocked, toBeBlocked;
  private boolean wRRookMoved = false, wLRookMoved = false, wKingMoved = false, bRRookMoved = false, bLRookMoved = false, bKingMoved = false;
  private boolean queenCastle = false, kingCastle = false;
  private int xOfSelectedPiece, yOfSelectedPiece, selectedPiece;
  private Set<String> movesPossible, checkPath, containsPieces, containsOpponentPieces, pawnDiagonal, stalemate;

  public int getImageHeight()
  {
    return boardimage.getIconHeight();
  }

  public int getImageWidth()
  {
    return boardimage.getIconWidth();
  }

  public boolean getCanQueenSide()
  {
    return queenCastle;
  }

  public boolean getCanKingSide()
  {
    return kingCastle;
  }

  public ChessJPanel()
  {
    b = new Board();
    enpassantEligible = new ArrayList<Integer>();
    enpassantEligibleRow = new ArrayList<Integer>();
    boardimage = new ImageIcon("board.png");

    setLayout(new GridLayout(8, 8));

    int col = 0, row = 0;

    for(int i = 0; i < 64; i++)
    {
      button = new JButton(); //one button for each square on the chessboard
      button.addActionListener(new ButtonHandler());
      button.setOpaque(false);
      button.setName(String.valueOf(col++)+"-"+String.valueOf(row));
      if (col == 8)
      {
        col = 0;
        row++;
      }
      button.setContentAreaFilled(false);
      button.setBorderPainted(false);
      this.add(button);
      setupGame();
    }
  }

  private void setupGame()
  {
    player = 1;             //1 if it's white's move, -1 if it's black's move
    playerIsMoving = false; //player has selected a piece to move
    moveSuccess = false;    //player has completed their move
    xOfSelectedPiece = 0;   //to keep track of where the clicked piece is
    yOfSelectedPiece = 0;   //to keep track of where the clicked piece is
    selectedPiece = 0;      //to keep track of what piece the player clicked
  }


  private class ButtonHandler implements ActionListener
  {
    public boolean movePawn(Board b, int player, int selectedPiece, int row, int col, int yOfSelected, int xOfSelectedPiece)
    {

      boolean enpassantIt = ((enpassantEligible.contains(xOfSelectedPiece + 1) ||enpassantEligible.contains(xOfSelectedPiece - 1)) && col == enpassantEligible.get(0) && b.get(col, row + player) == player*-1);

      if(row == yOfSelectedPiece - selectedPiece &&
        ((col == xOfSelectedPiece && b.get(col, row) == 0) ||
        ((col == xOfSelectedPiece + 1 || col == xOfSelectedPiece - 1) &&
        ((b.get(col, row)*player < 0) || enpassantIt))) && row == yOfSelectedPiece - player)
      {

        if(enpassantIt)
        {
          b.set(col, row + player, 0);
        }

        enpassantEligible.clear();
        // enpassantEligibleRow.clear();
        return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
      } //one-step move
      if(col == xOfSelectedPiece &&
         row == yOfSelectedPiece - selectedPiece*2 &&
         (yOfSelectedPiece == 1 || yOfSelectedPiece == 6)
         && (b.get(col, row) == 0 && b.get(col, yOfSelectedPiece - selectedPiece) == 0))
      {

        if (col < 7 && b.get(col+1, row) == player*-1)
        {
          enpassantEligible.add(col);
          // enpassantEligibleRow(row - player);
        }

        if (col > 0 && b.get(col-1,row) == player*-1)
        {
          enpassantEligible.add(col);
          // enpassantEligibleRow(row - player);
        }

        return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
      } //two-step move
      return false;
    }
    public boolean moveRook(Board b, int player, int selectedPiece, int row, int col, int yOfSelected, int xOfSelectedPiece, boolean blocked, boolean toBeBlocked)
    {
      if(col == xOfSelectedPiece &&
         row == yOfSelectedPiece) //prevent attacking itself
        blocked = true;
      for(int i = -7; i < 8; i++)
      {
        if((col == xOfSelectedPiece &&
            row == yOfSelectedPiece - i) ||
           (col == xOfSelectedPiece - i &&
            row == yOfSelectedPiece) && i != 0)
        {
          if(col != xOfSelectedPiece) //horizontal movement
          {
            for(int j = xOfSelectedPiece+1; j <= col; j++)
            {
              if(b.get(j, row)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, row)*player < 0)
                toBeBlocked = true;
            } //right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece-1; j >= col; j--)
            {
              if(b.get(j, row)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, row)*player < 0)
                toBeBlocked = true;
            } //left, checking if you can move where you clicked
          }
          if(row != yOfSelectedPiece) //vertical movement
          {
            for(int j = yOfSelectedPiece+1; j <= row; j++)
            {
              if(b.get(col, j)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(col, j)*player < 0)
                toBeBlocked = true;
            } //downward, checking if you can move where you clicked
            for(int j = yOfSelectedPiece-1; j >= row; j--)
            {
              if(b.get(col, j)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(col, j)*player < 0)
                toBeBlocked = true;
            } //upward, checking if you can move where you clicked
          }
          if(!blocked)  //movement of piece, if allowed
          {
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          }
        }
      }
      return false;
    }
    public boolean moveKnight(Board b, int player, int row, int col, int yOfSelected, int xOfSelectedPiece)
    {
      for(int i = -2; i < 3; i++)
      {
        for(int j = -2; j < 3; j++)
        {
          if(col == xOfSelectedPiece + i && i != 0 && j != 0
             && row == yOfSelectedPiece + j && i != j &&
             i * -1 != j
             && player*b.get(col, row) <= 0)
          {
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          } //movement of piece, if allowed
        }
      }
      return false;
    }
    public boolean moveBishop(Board b, int player, int selectedPiece, int row, int col, int yOfSelected, int xOfSelectedPiece, boolean blocked, boolean toBeBlocked)
    {
      if(col == xOfSelectedPiece &&
         row == yOfSelectedPiece) //prevent attacking itself
        blocked = true;
      for(int i = -7; i < 8; i++)
      {
        if((col == xOfSelectedPiece - i &&
            row == yOfSelectedPiece - i) ||
           (col == xOfSelectedPiece + i &&
            row == yOfSelectedPiece - i) && i != 0)
        {
          if(col - xOfSelectedPiece ==
             row - yOfSelectedPiece)
          {
            for(int j = xOfSelectedPiece+1; j <= col; j++)
            {
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //up-right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece-1; j >= col; j--)
            {
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //down-left, checking if you can move where you clicked
          }
          if(col - xOfSelectedPiece ==
             (row - yOfSelectedPiece)*-1)
          {
            for(int j = xOfSelectedPiece+1; j <= col; j++)
            {
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //down-right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece-1; j >= col; j--)
            {
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //up-left, checking if you can move where you clicked
          }
          if(!blocked)
          {
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          } //movement of piece, if allowed
        }
      }
      return false;
    }
    public boolean moveQueen(Board b, int player, int selectedPiece, int row, int col, int yOfSelected, int xOfSelectedPiece, boolean blocked, boolean toBeBlocked)
    {
      if(col == xOfSelectedPiece &&
         row == yOfSelectedPiece) //prevent attacking itself
        blocked = true;
      for(int i = -7; i < 8; i++)
      {
        if((col == xOfSelectedPiece &&
            row == yOfSelectedPiece - i) ||
           (col == xOfSelectedPiece - i &&
            row == yOfSelectedPiece) ||
           (col == xOfSelectedPiece - i &&
            row == yOfSelectedPiece - i) ||
           (col == xOfSelectedPiece + i &&
            row == yOfSelectedPiece - i) && i != 0)
        {
          if(row == yOfSelectedPiece) //horizontal movement
          {
            for(int j = xOfSelectedPiece + 1; j <= col; j++)
            {
              if(b.get(j, row)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, row)*player < 0)
                toBeBlocked = true;
            } //right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece - 1; j >= col; j--)
            {
              if(b.get(j, row)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, row)*player < 0)
                toBeBlocked = true;
            } //left, checking if you can move where you clicked
          }
          if(col == xOfSelectedPiece) //vertical movement
          {
            for(int j = yOfSelectedPiece + 1; j <= row; j++)
            {
              if(b.get(col, j)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(col, j)*player < 0)
                toBeBlocked = true;
            } //upward, checking if you can move where you clicked
            for(int j = yOfSelectedPiece - 1; j >= row; j--)
            {
              if(b.get(col, j)*player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(col, j)*player < 0)
                toBeBlocked = true;
            } //downward, checking if you can move where you clicked
          }
          if(col - xOfSelectedPiece ==
             row - yOfSelectedPiece)
          {
            for(int j = xOfSelectedPiece + 1; j <= col; j++)
            {
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //up-right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece - 1; j >= col; j--)
            {
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece + j - xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //down-left, checking if you can move where you clicked
          }
          if(col - xOfSelectedPiece ==
             (row - yOfSelectedPiece)*-1)
          {
            for(int j = xOfSelectedPiece + 1; j <= col; j++)
            {
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //down-right, checking if you can move where you clicked
            for(int j = xOfSelectedPiece - 1; j >= col; j--)
            {
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player > 0 || toBeBlocked)
                blocked = true;
              if(b.get(j, yOfSelectedPiece - j + xOfSelectedPiece)
                 *player < 0)
                toBeBlocked = true;
            } //up-left, checking if you can move where you clicked
          }
          if(!blocked)
          {
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          } //movement of piece, if allowed
        }
      }
      return false;
    }
    public boolean moveKing(Board b, int player, int row, int col, int yOfSelected, int xOfSelectedPiece)
    {
      if(queenCastle && (isInCheck(b, player*-1) == 0))
      {
        if(player > 0)
        {
          if(row == 7 && col == 2)
          {
            moveRook(b, player, 2, 7, 3, 7, 0, false, false);
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          }
        }
        else
        {
          if(row == 0 && col == 2)
          {
            moveRook(b, player, 2, 0, 3, 0, 0, false, false);
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          }
        }
      }
      if(kingCastle && (isInCheck(b, player*-1) == 0))
      {
        if(player > 0)
        {
          if(row == 7 && col == 6)
          {
            moveRook(b, player, 2, 7, 5, 7, 7, false, false);
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          }
        }
        else
        {
          if(row == 0 && col == 6)
          {
            moveRook(b, player, 2, 0, 5, 0, 7, false, false);
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          }
        }
      }

      for(int i = -1; i < 2; i++)
      {
        for(int j = -1; j < 2; j++)
        {
          if(col == xOfSelectedPiece + i &&
             row == yOfSelectedPiece + j && (i != 0 || j != 0)
             && player*b.get(col, row) <= 0)
          {
            return b.makeMove(col, row, xOfSelectedPiece, yOfSelectedPiece);
          } //movement of piece, if allowed
        }
      }
      return false;
    }

    public boolean isQueensideCastlePossible(Board b, int player, boolean wLRookMoved, boolean wKingMoved,
        boolean bLRookMoved, boolean bKingMoved) {
      if (player > 0) {
        if (b.get(3, 7) == 0 && b.get(2, 7) == 0 && b.get(1, 7) == 0 && b.get(0, 7) == 2
            && (!wLRookMoved && !wKingMoved)) {
          return true;
        }
        return false;
      } else {
        if (b.get(3, 0) == 0 && b.get(2, 0) == 0 && b.get(1, 0) == 0 && b.get(0, 0) == -2
            && (!bLRookMoved && !bKingMoved)) {
          return true;
        }
        return false;
      }
    }

    public boolean isKingsideCastlePossible(Board b, int player, boolean wRRookMoved, boolean wKingMoved,
        boolean bRRookMoved, boolean bKingMoved) {
      if (player > 0) {
        if (b.get(5, 7) == 0 && b.get(6, 7) == 0 && b.get(7, 7) == 2 && (!wRRookMoved && !wKingMoved)) {
          return true;
        }
        return false;
      } else {
        if (b.get(5, 0) == 0 && b.get(6, 0) == 0 && b.get(7, 0) == -2 && (!bRRookMoved && !bKingMoved)) {
          return true;
        }
        return false;
      }
    }

    public void actionPerformed(ActionEvent e)
    {
      //the below functions control both selection of a piece, and the
      //movement of a selected piece. see Board.java for piece values
      button = (JButton)e.getSource();
      blocked = false;      //if movement in a direction is blocked by a piece
      toBeBlocked = false;  //to make sure only opponent's pieces can be taken

      // get row and column of button pressed
      String [] rowcol = button.getName().split("-");
      int col = Integer.parseInt(rowcol[0]);
      int row = Integer.parseInt(rowcol[1]);
      // .getY() = row, .getX() = col

      queenCastle = isQueensideCastlePossible(b, player, wLRookMoved, wKingMoved, bLRookMoved, bKingMoved);
      kingCastle = isKingsideCastlePossible(b, player, wRRookMoved, wKingMoved, bRRookMoved, bKingMoved);

      if(playerIsMoving)  //if player has already selected a piece
      {
        if((selectedPiece == 1 || selectedPiece == -1)
           && player == selectedPiece)  //player selected pawn
        {
          moveSuccess = movePawn(b, player, selectedPiece, row, col, yOfSelectedPiece, xOfSelectedPiece);
        }
        if((selectedPiece == 2 || selectedPiece == -2)
           && player == selectedPiece/2)  //player selected rook
        {
          moveSuccess = moveRook(b, player, selectedPiece, row, col, yOfSelectedPiece, xOfSelectedPiece, blocked, toBeBlocked);
          if (moveSuccess) {
            if (player == 1) {
              if (row == 7 && col == 7)
                wRRookMoved = true;
              if (row == 0 && col == 7)
                wLRookMoved = true;
            } else {
              if (row == 7 && col == 0)
                bRRookMoved = true;
              if (row == 0 && col == 0)
                bLRookMoved = true;
            }
          }
        }
        if((selectedPiece == 3 || selectedPiece == -3)
           && player == selectedPiece/3)  //player selected knight
        {
          moveSuccess = moveKnight(b, player, row, col, yOfSelectedPiece, xOfSelectedPiece);
        }
        if((selectedPiece == 4 || selectedPiece == -4)
           && player == selectedPiece/4)  //player selected bishop
        {
          moveSuccess = moveBishop(b, player, selectedPiece, row, col, yOfSelectedPiece, xOfSelectedPiece, blocked, toBeBlocked);
        }
        if((selectedPiece == 5 || selectedPiece == -5)
           && player == selectedPiece/5)  //player selected queen
        {
          moveSuccess = moveQueen(b, player, selectedPiece, row, col, yOfSelectedPiece, xOfSelectedPiece, blocked, toBeBlocked);
        }
        if((selectedPiece == 6 || selectedPiece == -6)
           && player == selectedPiece/6)  //player selected king
        {
          moveSuccess = moveKing(b, player, row, col, yOfSelectedPiece, xOfSelectedPiece);
        }
      }

      if(!moveSuccess)
      {
        xOfSelectedPiece = col;
        yOfSelectedPiece = row;
        playerIsMoving = true;  //player has selected their piece
        selectedPiece = b.get(xOfSelectedPiece, yOfSelectedPiece);
        if(selectedPiece == 0)  //if they unselect by clicking an empty square
          playerIsMoving = false;
      } //if player has not moved yet, but opponent is selecting a piece
      else
      {
        if(isInCheck(b, player*-1) == 1) //check if you put yourself in check
        {
          moveSuccess = false;
          //undo previous move
          b.setBoard(prevb);
        }
        if(moveSuccess)
        {
          playerIsMoving = false; //player needs to select piece before moving

          if ((selectedPiece == 1 || selectedPiece == -1) && (row == 7 || row == 0))
          {
            repaint();
            int r = Chess.raisePromotion(player);
            b.set(col, row, r);
          }

          int isInCheck = isInCheck(b, player);

          if(isInCheck == 2 || isInCheck == 3)  //check if opponent is in checkmate
          {
            repaint();
            int r = 0;
            if(isInCheck == 2)
            {
              r = Chess.raiseCheckmate(player);
            }
            if(isInCheck == 3)
            {
              r = Chess.raiseStalemate(player);
              //display message with stalemate
            }
            if (r == 0)
              b.restartGame();
            else
              // not working for some reason??
              Chess.disposeJFrame();

            // player is switched as part of this if, make sure it's switched to white if we restart
            player = -1;
          }

          if (selectedPiece == 6 || selectedPiece == -6) {
          if (player == 1) {
            wKingMoved = true;
          } else {
            bKingMoved = true;
          }
        }

        if (selectedPiece != player) {
          enpassantEligible.clear();
          enpassantEligibleRow.clear();
        }

          player *= -1;   //switch to other player, from 1 to -1 or from -1 to 1
          moveSuccess = false;
          xOfSelectedPiece = 0;
          yOfSelectedPiece = 0;
          selectedPiece = 0;
        }
      }

      repaint();
      prevb = b.getBoardCopy();
    }
  }

  public void pawnPossible(Board b, int col, int row, int player, Set<String> movesPossible, Set<String> checkPath, int opponentKingCol, int opponentKingRow) {
    if (col - 1 >= 0 && row-player < 8 && row-player >= 0)
    {
      if(b.get(col - 1, row - player) * player < 0) {
        movesPossible.add(String.valueOf(col-1) + "-" + String.valueOf(row-player));
        if(col - 1 == opponentKingCol && row - player == opponentKingRow)
          checkPath.add(String.valueOf(col) + "-" + String.valueOf(row));
      }
      pawnDiagonal.add(String.valueOf(col - 1) + "-" + String.valueOf(row - player));
    }
    if (col + 1 < 8 && row-player < 8 && row-player >= 0)
    {
      if(b.get(col + 1, row - player) * player < 0) {
        movesPossible.add(String.valueOf(col+1) + "-" + String.valueOf(row-player));
        if(col + 1 == opponentKingCol && row - player == opponentKingRow)
          checkPath.add(String.valueOf(col) + "-" + String.valueOf(row));
      }
      pawnDiagonal.add(String.valueOf(col + 1) + "-" + String.valueOf(row - player));
    }
    if (row - player >= 0 && row - player < 8 && b.get(col, row-player) == 0)
      movesPossible.add(String.valueOf(col) + "-" + String.valueOf(row-player));
    if (((row == 1 && player == -1) || (row == 6 && player == 1)) && b.get(col, row-2*player) == 0 && b.get(col, row-player) == 0)
      movesPossible.add(String.valueOf(col) + "-" + String.valueOf(row-2*player));
  } //used for check
  public void knightPossible(Board b, int col, int row, int player, Set<String> movesPossible, Set<String> checkPath, int opponentKingCol, int opponentKingRow) {
    for (int i = -2; i <= 2; i += 1) {
      for (int j = -2; j <= 2; j += 1) {
        if ((i - j == 1 || i - j == -1 || i - j == 3 || i - j == -3) && i != 0 && j != 0) {
          if (col + i < 8 && col + i >= 0 && row + j < 8 && row + j >= 0)
          {
            movesPossible.add(String.valueOf(col+i) + "-" + String.valueOf(row+j));
            if(col + i == opponentKingCol && row + j == opponentKingRow)
              checkPath.add(String.valueOf(col) + "-" +  String.valueOf(row));
          }
        }
      }
    }
  } //used for check
  public void UDLRPossible(Board b, int col, int row, int player, Set<String> movesPossible,
  Set<String> checkPath, int opponentKingCol, int opponentKingRow) {
    int i = 1;
    while (col + i < 8 && col + i >= 0) {
      movesPossible.add(String.valueOf(col+i) + "-" + String.valueOf(row));
      if (col + i == opponentKingCol && row == opponentKingRow)
      {
        for(int k = col; k < opponentKingCol; k++)
          checkPath.add(String.valueOf(k) + "-" + String.valueOf(row));
      }
      if (b.get(col + i, row) * player != 0 && b.get(col + i, row) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (col - i < 8 && col - i >= 0) {
      movesPossible.add(String.valueOf(col-i) + "-" + String.valueOf(row));
      if (col - i == opponentKingCol && row == opponentKingRow)
      {
        for(int k = col; k > opponentKingCol; k--)
          checkPath.add(String.valueOf(k) + "-" + String.valueOf(row));
      }
      if (b.get(col - i, row) * player != 0 && b.get(col - i, row) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (row + i < 8 && row + i >= 0) {
      movesPossible.add(String.valueOf(col) + "-" + String.valueOf(row+i));
      if (col == opponentKingCol && row + i == opponentKingRow)
      {
        for(int k = row; k < opponentKingRow; k++)
          checkPath.add(String.valueOf(col) + "-" + String.valueOf(k));
      }
      if (b.get(col, row + i) * player != 0 && b.get(col, row + i) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (row - i < 8 && row - i >= 0) {
      movesPossible.add(String.valueOf(col) + "-" + String.valueOf(row-i));
      if (col == opponentKingCol && row - i == opponentKingRow)
      {
        for(int k = row; k > opponentKingRow; k--)
          checkPath.add(String.valueOf(col) + "-" + String.valueOf(k));
      }
      if (b.get(col, row - i) * player != 0 && b.get(col, row - i) != player*-6)
        break;
      i++;
    }
  } //used for check

  public void diagonalsPossible(Board b, int col, int row, int player, Set<String> movesPossible, Set<String> checkPath, int opponentKingCol, int opponentKingRow) {
    int i = 1;
    while (col + i < 8 && col + i >= 0 && row + i < 8 && row + i >= 0) {
      movesPossible.add(String.valueOf(col+i) + "-" + String.valueOf(row+i));
      if (col + i == opponentKingCol && row + i == opponentKingRow)
      {
        for(int k = row; k < opponentKingRow; k++)
          checkPath.add(String.valueOf(col-row+k) + "-" + String.valueOf(k));
      }
      if (b.get(col + i, row + i) * player != 0 && b.get(col + i, row + i) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (col - i < 8 && col - i >= 0 && row + i < 8 && row + i >= 0) {
      movesPossible.add(String.valueOf(col-i) + "-" + String.valueOf(row+i));
      if (col - i == opponentKingCol && row + i == opponentKingRow)
      {
        for(int k = row; k < opponentKingRow; k++)
          checkPath.add(String.valueOf(col+row-k) + "-" + String.valueOf(k));
      }
      if (b.get(col - i, row + i) * player != 0 && b.get(col - i, row + i) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (col + i < 8 && col + i >= 0 && row - i < 8 && row - i >= 0) {
      movesPossible.add(String.valueOf(col+i) + "-" + String.valueOf(row-i));
      if (col + i == opponentKingCol && row - i == opponentKingRow)
      {
        for(int k = row; k > opponentKingRow; k--)
          checkPath.add(String.valueOf(col+row-k) + "-" + String.valueOf(k));
      }
      if (b.get(col + i, row - i) * player != 0 && b.get(col + i, row - i) != player*-6)
        break;
      i++;
    }
    i = 1;
    while (col - i < 8 && col - i >= 0 && row - i < 8 && row - i >= 0) {
      movesPossible.add(String.valueOf(col-i) + "-" + String.valueOf(row-i));
      if (col - i == opponentKingCol && row - i == opponentKingRow)
      {
        for(int k = row; k > opponentKingRow; k--)
          checkPath.add(String.valueOf(col-row+k) + "-" + String.valueOf(k));
      }
      if (b.get(col - i, row - i) * player != 0 && b.get(col - i, row - i) != player*-6)
        break;
      i++;
    }
  } //used for check

  public void kingPossible(Board b, int col, int row, int player, Set<String> movesPossible) {
    if(b.get(col, row) == player*6)
      {
        for(int i = -1; i <= 1; i++)
        {
          for(int j = -1; j <= 1; j++)
          {
            if(col + i < 8 && col + i >= 0 &&
               row + j < 8 && row + j >= 0
               && (i != 0 || j != 0))
              movesPossible.add(String.valueOf(col+i) + "-" + String.valueOf(row+j));
          }
        }
      }
  } //used for check

  public void cycleBoard(Board b, int player, int opponentKingCol, int opponentKingRow, boolean includeKing)
  {
    for (int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        if(b.get(i, j) == player*-1 || b.get(i, j) == player*-2 || b.get(i, j) == player*-3 ||
           b.get(i, j) == player*-4 || b.get(i, j) == player*-5)
        {
          containsOpponentPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
        if(b.get(i, j) == player)
        {
          pawnPossible(b, i, j, player, movesPossible, checkPath, opponentKingCol, opponentKingRow);
          containsPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
        if(b.get(i, j) == player*2 ||
           b.get(i, j) == player*5)
        {
          UDLRPossible(b, i, j, player, movesPossible, checkPath, opponentKingCol, opponentKingRow);
          containsPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
        if(b.get(i, j) == player*3)
        {
          knightPossible(b, i, j, player, movesPossible, checkPath, opponentKingCol, opponentKingRow);
          containsPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
        if(b.get(i, j) == player*4 ||
           b.get(i, j) == player*5)
        {
          diagonalsPossible(b, i, j, player, movesPossible, checkPath, opponentKingCol, opponentKingRow);
          containsPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
        if(b.get(i, j) == player*6 && includeKing)
        {
          kingPossible(b, i, j, player, movesPossible);
          containsPieces.add(String.valueOf(i) + "-" + String.valueOf(j));
        }
      }
    }
  }

  public int isInCheck(Board b, int player) //checks if player is in check
  {
    movesPossible = new HashSet<String>();
    checkPath = new HashSet<String>();
    containsPieces = new HashSet<String>();
    containsOpponentPieces = new HashSet<String>();
    pawnDiagonal = new HashSet<String>();
    Set<String> containsPiecesCopy=  new HashSet<String>();
    Set<String> initialCheckmateCheck = new HashSet<String>();
    Set<String> opponentKingMoves = new HashSet<String>();
    Set<String> checkPathCopy = new HashSet<String>();
    Set<String> movesPossibleCopy = new HashSet<String>();
    Set<String> containsOpponentPiecesCopy= new HashSet<String>();
    int[][] checktest;
    String opponentKingSquare = "";
    int opponentKingCol = 0, opponentKingRow = 0;
    boolean onlyKings = true;
    //cycles through board and adds each spot "covered" by each of the player's pieces to a set

    for(int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        if(b.get(i, j) != 6 && b.get(i, j) != -6 && b.get(i, j) != 0)
          onlyKings = false;
      }
    }

    if(onlyKings)
      return 3;

    for(int i = 0; i < 8; i++)  //DO NOT combine this with the cycleBoard function
    {
      for(int j = 0; j < 8; j++)
      {
        if(b.get(i, j) == player*-6)
          {
            kingPossible(b, i, j, player*-1, opponentKingMoves);

            opponentKingCol = i;
            opponentKingRow = j;
            opponentKingSquare = (String.valueOf(i) + "-" + String.valueOf(j));
          }
      }
    }
    cycleBoard(b, player, opponentKingCol, opponentKingRow, true);

    //adds each spot around the king to a set
    for(int j = opponentKingCol - 1; j < opponentKingCol+2; j++)
    {
      for(int i = opponentKingRow - 1; i < opponentKingRow+2; i++)
      {
          if (i >= 0 && j >= 0 && j < 8 && i < 8 && (j != opponentKingCol || i != opponentKingRow))
            opponentKingMoves.add(String.valueOf(j) + "-" + String.valueOf(i));
            // king can move here
      }
    }

    initialCheckmateCheck.addAll(containsOpponentPieces);
    initialCheckmateCheck.addAll(movesPossible);
    initialCheckmateCheck.addAll(containsPieces);
    initialCheckmateCheck.addAll(pawnDiagonal);
    movesPossibleCopy.addAll(movesPossible);
    containsOpponentPiecesCopy.addAll(containsOpponentPieces);

    if(movesPossible.contains(opponentKingSquare) && initialCheckmateCheck.containsAll(opponentKingMoves))
    {
      //all the code below checks for a checkmate
      containsPiecesCopy.addAll(containsPieces);
      checkPathCopy.addAll(checkPath);
      opponentKingMoves.removeAll(movesPossible);
      opponentKingMoves.removeAll(containsOpponentPieces);
      opponentKingMoves.removeAll(pawnDiagonal);
      movesPossible.clear();
      cycleBoard(b, player*-1, opponentKingCol, opponentKingRow, false);
      movesPossible.retainAll(checkPathCopy);
      opponentKingMoves.addAll(movesPossible);
      containsPiecesCopy.retainAll(movesPossibleCopy);
      containsPiecesCopy.removeAll(movesPossible);
      opponentKingMoves.removeAll(containsPiecesCopy);
      if(opponentKingMoves.size() == 0)
        return 2;
    }

    if(movesPossibleCopy.contains(opponentKingSquare))
      return 1; //check

    opponentKingMoves.clear();
    for(int j = opponentKingCol - 1; j < opponentKingCol+2; j++)
    {
      for(int i = opponentKingRow - 1; i < opponentKingRow+2; i++)
      {
          if (i >= 0 && j >= 0 && j < 8 && i < 8 && (j != opponentKingCol || i != opponentKingRow))
            opponentKingMoves.add(String.valueOf(j) + "-" + String.valueOf(i));
            // king can move here
      }
    }

    opponentKingMoves.removeAll(movesPossibleCopy);
    opponentKingMoves.removeAll(containsOpponentPiecesCopy);
    opponentKingMoves.removeAll(pawnDiagonal);

    if(opponentKingMoves.size() == 0)
    {
      movesPossible.clear();
      cycleBoard(b, player*-1, opponentKingCol, opponentKingRow, false);
      movesPossible.removeAll(containsOpponentPiecesCopy);
      stalemate = new HashSet<String>();
      stalemate.addAll(movesPossible);
      for(int i = 0; i < 8; i++)
      {
        for(int j = 0; j < 8; j++)
        {
          if(b.get(i, j) == player*-1 && j-player*-1 < 8 && j-player*-1 >= 0 && b.get(i, j-player*-1) == 0)
          {
            checktest = b.getBoardCopy();
            b.set(i, j, 0);
            if(isInCheck(b, player) == 1)
              stalemate.remove(String.valueOf(i) + "-" + String.valueOf(j-player*-1));
            b.setBoard(checktest);
          }
          if(b.get(i, j) == player*-6)
            stalemate.remove(String.valueOf(i) + "-" + String.valueOf(j));
        }
      }
      if(stalemate.size() == 0)
        return 3; //stalemate
    }

    return 0; //nothing
  }

  public void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    boardimage.paintIcon(this, g, 0, 0);
    for(int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        if(b.get(i, 7-j) != 0)
          b.getImage(i, 7-j).paintIcon(this, g, 75*i + 7, 533 - 75*j);
      } //images of the 32 pieces
    }

    if(playerIsMoving)  //all the code below is for the red guide squares
    {
      // get row and column of button pressed
      String [] rowcol = button.getName().split("-");
      int col = Integer.parseInt(rowcol[0]);
      int row = Integer.parseInt(rowcol[1]);
      // .getY() = row, .getX() = col

      g.setColor(Color.RED);
      if(b.get(col, row) == player &&
         row-player < 8 && row-player >= 0)
      {
        if(b.get(col, row-player) == 0)
          g.fillRect(button.getX()+30, button.getY()+30-75*player, 15, 15);
        if((row == 1 || row == 6) && row-2*player >= 0 && row-2*player < 8 && (b.get(col, row-2*player) == 0 && b.get(col, row-player) == 0))
          g.fillRect(button.getX()+30, button.getY()+30-150*player, 15, 15);
        if(col - 1 >= 0 && (b.get(col-1, row-player)*player < 0 || (enpassantEligible.contains(col-1) && b.get(col-1, row) == player*-1)))
          g.fillRect(button.getX()-45, button.getY()+30-75*player, 15, 15);
        if(col + 1 < 8 && (b.get(col+1, row-player)*player < 0 || (enpassantEligible.contains(col+1) && b.get(col+1, row) == player*-1)))
          g.fillRect(button.getX()+105, button.getY()+30-75*player, 15, 15);
      } //guide squares for pawn
      if(b.get(col, row) == player*2 ||
         b.get(col, row) == player*5)
      {
        int i = 1;
        while(col + i < 8 && col + i >= 0 && b.get(col + i, row)*player <= 0)
        {
          g.fillRect(button.getX() + 30 + 75*i, button.getY() + 30, 15, 15);
          if(b.get(col + i, row)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(col - i < 8 && col - i >= 0 && b.get(col - i, row)*player <= 0)
        {
          g.fillRect(button.getX() + 30 - 75*i, button.getY() + 30, 15, 15);
          if(b.get(col - i, row)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(row + i < 8 && row + i >= 0 && b.get(col, row + i)*player <= 0)
        {
          g.fillRect(button.getX() + 30, button.getY() + 30 + 75*i, 15, 15);
          if(b.get(col, row + i)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(row - i < 8 && row - i >= 0 && b.get(col, row - i)*player <= 0)
        {
          g.fillRect(button.getX() + 30, button.getY() + 30 - 75*i, 15, 15);
          if(b.get(col, row - i)*player < 0)
            break;
          i++;
        }
      } //guide squares for rook and queen's horizontal/vertical moves
      if(b.get(col, row) == player*3)
      {
        for(int i = -120; i <= 180; i += 75)
        {
          for(int j = -120; j <= 180; j += 75)
          {
            if((i - j == 75 || i - j == -75 || i - j == 225 || i - j == -225) &&
               i != 30 && j != 30)
            {
              if(col + (i-30)/75 < 8 && col + (i-30)/75 >= 0 &&
                 row + (j-30)/75 < 8 && row + (j-30)/75 >= 0 &&
                 b.get(col+(i-30)/75, row+(j-30)/75)*player <= 0)
                g.fillRect(button.getX() + i, button.getY() + j, 15, 15);
            }
          }
        }
      } //guide squares for knight
      if(b.get(col, row) == player*4 ||
         b.get(col, row) == player*5)
      {
        int i = 1;
        while(col+i < 8 && col+i >= 0 &&
              row+i < 8 && row+i >= 0 &&
              b.get(col+i, row+i)*player<=0)
        {
          g.fillRect(button.getX()+30+75*i, button.getY()+30+75*i, 15, 15);
          if(b.get(col+i,row+i)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(col-i < 8 && col-i >= 0 &&
              row+i < 8 && row+i >= 0 &&
              b.get(col-i, row+i)*player<=0)
        {
          g.fillRect(button.getX()+30-75*i, button.getY()+30+75*i, 15, 15);
          if(b.get(col-i,row+i)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(col+i < 8 && col+i >= 0 &&
              row-i < 8 && row-i >= 0 &&
              b.get(col+i, row-i)*player<=0)
        {
          g.fillRect(button.getX()+30+75*i, button.getY()+30-75*i, 15, 15);
          if(b.get(col+i,row-i)*player < 0)
            break;
          i++;
        }
        i = 1;
        while(col-i < 8 && col-i >= 0 &&
              row-i < 8 && row-i >= 0 &&
              b.get(col-i, row-i)*player<=0)
        {
          g.fillRect(button.getX()+30-75*i, button.getY()+30-75*i, 15, 15);
          if(b.get(col-i,row-i)*player < 0)
            break;
          i++;
        }
      } //guide squares for bishop and queen's diagonal moves
      if(b.get(col, row) == player*6)
      {
        if (getCanQueenSide()) {
          if (player > 0) {
            g.fillRect(button.getX() - 120, button.getY() + 30, 15, 15);
          } else {
            g.fillRect(button.getX() - 120, button.getY()+ 30, 15, 15);
          }
        }
        if (getCanKingSide()) {
          if (player > 0) {
            g.fillRect(button.getX() + 180, button.getY() + 30, 15, 15);
          } else {
            g.fillRect(button.getX() + 180, button.getY() + 30, 15, 15);
          }
        }

        for(int i = -45; i <= 105; i += 75)
        {
          for(int j = -45; j <= 105; j += 75)
          {
            if(col + (i-30)/75 < 8 && col + (i-30)/75 >= 0 &&
               row + (j-30)/75 < 8 && row + (j-30)/75 >= 0 &&
               b.get(col+(i-30)/75, row+(j-30)/75)*player <= 0
               && (i != 30 || j != 30))
              g.fillRect(button.getX() + i, button.getY() + j, 15, 15);
          }
        }
      } //guide squares for king
    }
  }
}

class dialogMessages {
  static public int displayCheckmate(JFrame frame, int player, boolean draw) {
    Object[] options = { "Restart", "Quit" };
    player = player < 0 ? 1 : 2;

    String message = draw ? "Game has ended in a draw." : "Checkmate. Player " + player + " has won!\n" + "Would you like to play again?";

    return JOptionPane.showOptionDialog(frame,
        message, "Game over!",
        JOptionPane.YES_NO_CANCEL_OPTION, JOptionPane.INFORMATION_MESSAGE, null, options, options[0]);
  }

  static public int displayPromotion(JFrame frame, int player)
  {
    Object[] options = {"Queen", "Rook", "Bishop", "Knight"};

    return JOptionPane.showOptionDialog(frame,
        "Choose a piece to replace your pawn.",
        "Pawn Promotion",
        JOptionPane.DEFAULT_OPTION,
        JOptionPane.QUESTION_MESSAGE,
        null, options, options[0]);
  }
}
