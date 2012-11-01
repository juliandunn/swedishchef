/*
    chef.c
    ------
    This program will convert English from stdin or file to Mock Swedish,
    or Mock Chinese as the Swedes insist, on stdout.

    This program is a C conversion of the lex source by John Hagerman.

    Arjan Kenter, 16 Dec 1994
                  19 Dec 1994 LaTeX related bugs fixed (forgot to test...)
                  28 Feb 1996 HTML support included and newline bug fixed
                   4 Dec 1996 Added code to skip carriage returns so that the chef web
                              page will add Bork Bork Borks to input from PCs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INWORD "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'"
#define INTAG  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/"

#define BUFLEN 100 /* max number of chars to match + trailing context. */
                   /* chosen safely for future extensions, needs only be 5. */

char buf[BUFLEN];
int  head = 0;
int  tail = 0;
int  mtch = 0;
int  eofl = 0;

int nextchar (void)
  {
    int c;
    if (mtch == head)
      {
        if (eofl || (c = getchar ()) == EOF)
          {
            eofl = 1;
            return EOF;
          }
        buf[head++] = c;
        if (head == BUFLEN) head = 0;
        if (head == tail)
          {
            fprintf (stderr, "Chef internal error: buffer overflowed.\n");
            exit (EXIT_FAILURE);
          }
      }
    c = buf[mtch++];
    if (mtch == BUFLEN) mtch = 0;
    if (c == '\r') c = nextchar (); /* Skip carriage returns. I hope this simplicity suffices. */
    return c;
  }

int back (int c)
  {
    if (c != EOF && --mtch < 0) mtch = BUFLEN-1;
    return c;
  }

int match (char *str)
  {
    int c;
    unsigned char *s = (unsigned char *) str;

    mtch = tail;
    while (*s && *s == nextchar ()) s++;
    if (*s) mtch = tail;
    return !*s;
  }

int eof (void)
  {
    back (nextchar ());
    return eofl && head == tail;
  }

int eow (void)
  {
    return strchr (INWORD, back (nextchar ())) == NULL;
  }

int eol (void)
  {
    return back (nextchar ()) == '\n';
  }

int eotex (void)
  {
    int c = back (nextchar ());
    return c == ' ' || c == '\n';
  }

void accept (void)
  {
    tail = mtch;
  }

void echo (void)
  {
    int c;
    mtch = tail;
    if ((c = nextchar ()) != EOF)
      {
        accept ();
        putchar (c);
      }
  }

void echo_tag (void)
  {
    /* Echo everything upto and including next '>', where it */
    /* appears outside "" delimited strings. */

    int c;
    int instr = 0;
    mtch = tail;
    while ((c = nextchar ()) != EOF && (c != '>' || instr))
      {
        accept (); putchar (c);
        if (c == '"') instr = !instr;
      }
  }

void echo_esc (void)
  {
    /* Echo everything upto next non-alphanumeric character. */

    int c;
    echo (); /* Echo '&'. */
    if ((c = nextchar ()) != EOF && (isalnum (c) || c == '#'))
      {
        accept ();
        putchar (c);
      }
    while ((c = nextchar ()) != EOF && isalnum (c))
      {
        accept ();
        putchar (c);
      }
  }

void chef (void)
  {
    int c, icount = 0;
    int beginword, inword = 0;
    int bork = 1;
    while (!eof ())
      {
        /* Determine if we are at beginning of word, within word or
           outside word.
        */
        mtch = tail;
        beginword = strchr (INWORD, c = nextchar ()) && !inword;

        if (match ("\\")) /* TeX command encountered. */
          {
            do {echo ();} while (!eof () && !eotex ());
            inword = 0;
            continue;
          }

        if (match ("<") && strchr (INTAG, nextchar ())) /* HTML tag encountered. */
          {
            echo_tag ();
            inword = 0;
            continue;
          }

        if (match ("&") && (isalnum (c = nextchar ()) || c == '#')) /* HTML escape encountered. */
          {
            echo_esc ();
            inword = 0;
            continue;
          }

        /* Suppress "Bork bork bork!" after a line with '%' in it to
           avoid problems with comments in LaTeX files.
        */

        if (match ("%")) {echo (); bork = 0; continue;}
        if (eol ())      {echo (); bork = 1; inword = 0; continue;}

        if (beginword)
          {
            icount = 0;
            if (match ("e"))    {accept (); inword = 1; printf ("i"); continue;}
            if (match ("E"))    {accept (); inword = 1; printf ("I"); continue;}
            if (match ("o"))    {accept (); inword = 1; printf ("oo"); continue;}
            if (match ("O"))    {accept (); inword = 1; printf ("Oo"); continue;}
            if (match ("bork")) {accept (); inword = 1; printf ("bork"); continue;}
            if (match ("Bork")) {accept (); inword = 1; printf ("Bork"); continue;}
          }

        if (inword)
          {
            if (match ("ew"))   {accept (); inword = 1; printf ("oo"); continue;}
            if (match ("f"))    {accept (); inword = 1; printf ("ff"); continue;}
            if (match ("ir"))   {accept (); inword = 1; printf ("ur"); continue;}
            if (match ("ow"))   {accept (); inword = 1; printf ("oo"); continue;}
            if (match ("o"))    {accept (); inword = 1; printf ("u");  continue;}
            if (match ("u"))    {accept (); inword = 1; printf ("oo"); continue;}
            if (match ("U"))    {accept (); inword = 1; printf ("Oo"); continue;}
            if (match ("tion")) {accept (); inword = 1; printf ("shun"); continue;}
            if (match ("i"))    {accept (); inword = 1; printf (icount++ ? "i" : "ee"); continue;}
            if (match ("e")  && eow ()) {accept (); inword = 0; printf ("e-a"); continue;}
            if (match ("en") && eow ()) {accept (); inword = 0; printf ("ee"); continue;}
            if (match ("th") && eow ()) {accept (); inword = 0; printf ("t"); continue;}
          }

        if (match ("an"))  {accept (); inword = 1; printf ("un");  continue;}
        if (match ("An"))  {accept (); inword = 1; printf ("Un");  continue;}
        if (match ("au"))  {accept (); inword = 1; printf ("oo");  continue;}
        if (match ("Au"))  {accept (); inword = 1; printf ("Oo");  continue;}
        if (match ("the")) {accept (); inword = 1; printf ("zee"); continue;}
        if (match ("The")) {accept (); inword = 1; printf ("Zee"); continue;}
        if (match ("v"))   {accept (); inword = 1; printf ("f");   continue;}
        if (match ("V"))   {accept (); inword = 1; printf ("F");   continue;}
        if (match ("w"))   {accept (); inword = 1; printf ("v");   continue;}
        if (match ("W"))   {accept (); inword = 1; printf ("V");   continue;}
        if (match ("a") && !eow ()) {accept (); inword = 1; printf ("e"); continue;}
        if (match ("A") && !eow ()) {accept (); inword = 1; printf ("E"); continue;}
        if (match (".") &&  eol () && bork)
          {
            accept (); inword = 0; printf (".\nBork Bork Bork!");
            continue;
          }

        inword = strchr (INWORD, c) != NULL;
        echo ();
      }
  }

int main (int argc, char **argv)
  {
    if (argc > 2)
      {
        fprintf (stderr, "Usage: chef [file]\n");
        exit (1);
      }
    if (argc == 2)
      {
        if (freopen (argv[1], "r", stdin) == NULL)
          {
            fprintf (stderr, "Can't open '%s'.\n", argv[1]);
            exit (1);
          }
      }
    chef ();
    return 0;
  }
