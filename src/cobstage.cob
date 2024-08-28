      
      ******************************************************************
      *
      *   Name: cobstage.cob
      *         Sample POSIX Pipelines stage in COBOL
      *   Date: 2024-06-12 (Wed)
      *   Also: rxstage.rx, vpdemopy.py
      *
      * Comments start with * in Column 7
      * COBOL programs have a Section, Paragraph, Sentence, Statements structure.
      *
      ******************************************************************
      
       Identification Division.
       Program-ID. COBSTAGE.
       Environment DIVISION.
       Configuration Section.
      
       Data Division.
      
       Working-Storage Section.
       77 IBytes                 Pic S9(9) comp-5 Value 19.
       77 OBytes                 Pic S9(9) comp-5 Value 32.
       77 Result                 Pic S9(9) comp-5 Value 0.
       77 ic                     Pic S9(9) comp-5 Value 0.
       77 oc                     Pic S9(9) comp-5 Value 0.
       77 bc                     Pic S9(9) comp-5 Value 0.
       77 sn                     Pic S9(9) comp-5 Value 0.
       77 buflen                 Pic S9(9) comp-5 Value 0.
       77 Version-String         Pic X(36).
       77 buffer                 Pic X(256).

       Procedure Division.

        MAIN-PROCEDURE.

          Display 'POSIX Pipelines (XFL) demonstration stage in COBOL'.


          call 'XFLVERSN' using Version-String returning Result.
          Display 'POSIX Pipelines (XFL) version ' Version-String.

      * start with input record count of zero
          Move 0 to ic.
      * start with output record count of zero
          Move 0 to oc.
      * start with total byte count of zero
          Move 0 to bc.
      * we will be using stream 0
          Move 0 to sn.
      * start with a non-zero result
          Move 0 to Result.

          PERFORM PeekPutPurge UNTIL Result not = 0.
          Display 'input records:' ic
                  ' output records:' oc ' bytes:' bc.
          Move 0 to RETURN-CODE.
          stop run.

        PeekPutPurge.

          Move 255 to buflen.

          call 'XFLPEEK' using sn buffer buflen returning Result.
          if (Result not = 0) then perform END-OF-LOOP.
          Compute ic = ic + 1.

          call 'XFLOUT' using sn buffer buflen returning Result.
          if (Result not = 0) then perform END-OF-LOOP.
          Compute oc = oc + 1.
          Compute bc = bc + buflen.

          Move 255 to buflen.
          call 'XFLREAD' using sn buffer buflen returning Result.
          if (Result not = 0) then perform END-OF-LOOP.

       END-OF-LOOP.
          Display 'input records ' ic
               ' / output records ' oc ' / bytes ' bc.
          Move 0 to RETURN-CODE.
          stop run.

       End Program COBSTAGE.


