import javax.swing.*;

public class Board
{
  private int[][] chessboard;
  // if images fail to load, we only have to change the path here
  private String path = "";

  public Board()
  {
    chessboard = new int[8][8];
    setupBoard();
  }

  private void setupBoard()
  {
    //0 - empty
    //1 - pawn, 2 - rook, 3 - knight, 4 - bishop, 5 - queen, 6 - king
    //positive value for white, negative value for black

    for(int i = 0; i < 8; i++)
    {
      for(int j = 2; j < 6; j++)
        chessboard[i][j] = 0;
    }

    for(int i = 0; i < 8; i++)
    {
      chessboard[i][6] = 1;
      chessboard[i][1] = -1;
    }

    chessboard[0][7] = 2;
    chessboard[7][7] = 2;
    chessboard[0][0] = -2;
    chessboard[7][0] = -2;
    chessboard[1][7] = 3;
    chessboard[6][7] = 3;
    chessboard[1][0] = -3;
    chessboard[6][0] = -3;
    chessboard[2][7] = 4;
    chessboard[5][7] = 4;
    chessboard[2][0] = -4;
    chessboard[5][0] = -4;
    chessboard[3][7] = 5;
    chessboard[3][0] = -5;
    chessboard[4][7] = 6;
    chessboard[4][0] = -6;
  }

  public void restartGame()
  {
    setupBoard();
  }

  public boolean set(int x, int y, int p)
  {
    if(x < 0 || x > 7 || y < 0 || y > 7)
      return false;
    chessboard[x][y] = p;
    return true;
  }

  public boolean clear(int x, int y)
  {
    if(x < 0 || x > 7 || y < 0 || y > 7)
      return false;
    chessboard[x][y] = 0;
    return true;
  }

  public boolean makeMove(int col, int row, int xOfSelectedPiece, int yOfSelectedPiece)
  {
    int selectedPiece = get(xOfSelectedPiece, yOfSelectedPiece);
    if (clear(col, row) &&
        clear(xOfSelectedPiece, yOfSelectedPiece) &&
        set(col, row, selectedPiece) &&
        set(xOfSelectedPiece, yOfSelectedPiece, 0))
      return true;
    else
      return false;
  }

  public int get(int x, int y)
  {
    return chessboard[x][y];
  }

  public int[][] getBoardCopy()
  {
    int [][] newboard = new int[8][8];
    for (int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
        newboard[i][j] = chessboard[i][j];
    }
    return newboard;
  }

  public void setBoard(int [][] otherboard)
  {
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
        chessboard[i][j] = otherboard[i][j];
    }
  }

  public ImageIcon getImage(int x, int y)
  {
    if(chessboard[x][y] == 1)
      return new ImageIcon(path+"whitepawn.png");
    if(chessboard[x][y] == -1)
      return new ImageIcon(path+"blackpawn.png");
    if(chessboard[x][y] == 2)
      return new ImageIcon(path+"whiterook.png");
    if(chessboard[x][y] == -2)
      return new ImageIcon(path+"blackrook.png");
    if(chessboard[x][y] == 3)
      return new ImageIcon(path+"whiteknight.png");
    if(chessboard[x][y] == -3)
      return new ImageIcon(path+"blackknight.png");
    if(chessboard[x][y] == 4)
      return new ImageIcon(path+"whitebishop.png");
    if(chessboard[x][y] == -4)
      return new ImageIcon(path+"blackbishop.png");
    if(chessboard[x][y] == 5)
      return new ImageIcon(path+"whitequeen.png");
    if(chessboard[x][y] == -5)
      return new ImageIcon(path+"blackqueen.png");
    if(chessboard[x][y] == 6)
      return new ImageIcon(path+"whiteking.png");
    if(chessboard[x][y] == -6)
      return new ImageIcon(path+"blackking.png");
    return new ImageIcon("");
  }
}
