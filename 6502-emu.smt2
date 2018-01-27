; Emulator for 6502 in SMT2 format.
; This can be used for verifying compiler optimizations.
; It assumes that the sequence of instructions has one entry point
; at the first instruction. It stops executing if the sequence
; exits early because of a branch, jmp, rts, or rti.
; The only instructions not supported are jsr and brk, because they are
; implicit entry points.
;
; The equivalence checking can ignore variables are not live at the
; end of the sequence. The assertions assertALive etc. control this
; behavior.
;
; To prove two instruction sequences equivalent, write two functions
; which apply the instructions to the const someState. Then write
; (assert (not (sameState (candidate1 someState) (candidate2 someState))))
; This says -- there exists some initial machine state someState
; where candidate1 and candidate2 have different results. If the result
; is "unsat", then the solver has proven that the two sequences are
; equivalent for all possible machine states, given the set of registers
; and flags we care about at the end.
; If it returns "sat", then use (get-model) to see the initial state which
; is a counter-example, and use e.g. (eval (candidate1 someState)) to see
; what the function results in.

(define-sort Byte () (_ BitVec 8))
(define-sort Word () (_ BitVec 16))

(define-fun ext ((val Byte)) Word
  ((_ zero_extend 8) val)
)

(define-fun lobyte ((val Word)) Byte
  ((_ extract 7 0) val)
)

(define-fun hibyte ((val Word)) Byte
  ((_ extract 15 8) val)
)

(declare-datatypes (U T) ((Either (Left (left U))
                                  (Right (right T)))))

(declare-datatypes () ((SpecialExit End Rts Rti)))

(define-sort Exit () (Either SpecialExit Word))

(declare-datatypes ()
  ((Machine (mk-machine-private
    (a Byte)
    (x Byte)
    (y Byte)
    (sp Byte)
    (ccS Bool)
    (ccV Bool)
    (ccI Bool)
    (ccD Bool)
    (ccC Bool)
    (ccZ Bool)
    (mem (Array Word Byte))
    (exitLoc Exit))))
)

(define-fun mk-machine2 ((m Machine)
                         (a_ Byte)
                         (x_ Byte)
                         (y_ Byte)
                         (sp_ Byte)
                         (s_ Bool)
                         (v_ Bool)
                         (i_ Bool)
                         (d_ Bool)
                         (c_ Bool)
                         (z_ Bool)
                         (mem_ (Array Word Byte))
                         (exitLoc_ Exit)) Machine
  (ite (= (Left End) (exitLoc m))
    (mk-machine-private a_ x_ y_ sp_ s_ v_ i_ d_ c_ z_ mem_ exitLoc_)
    m))

(define-fun set-exit ((e Exit) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
   (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) e
  )
)

(define-fun set-SZ ((val Byte) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
   (bvuge val #x80) (ccV m) (ccI m) (ccD m) (ccC m) (= #x00 val) (mem m) (exitLoc m)
  )
)

(define-fun set-x ((newX Byte) (m Machine)) Machine
  (mk-machine2 m (a m) newX (y m) (sp m)
   (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-xSZ ((newX Byte) (m Machine)) Machine
  (mk-machine2 m (a m) newX (y m) (sp m)
   (bvuge newX #x80) (ccV m) (ccI m) (ccD m) (ccC m) (= #x00 newX) (mem m) (exitLoc m)
  )
)

(define-fun set-y ((newY Byte) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) newY (sp m)
    (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ySZ ((newY Byte) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) newY (sp m)
   (bvuge newY #x80) (ccV m) (ccI m) (ccD m) (ccC m) (= #x00 newY) (mem m) (exitLoc m)
  )
)


(define-fun set-a ((newA Byte) (m Machine)) Machine
  (mk-machine2 m newA (x m) (y m) (sp m)
    (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-aSZ ((newA Byte) (m Machine)) Machine
  (mk-machine2 m newA (x m) (y m) (sp m)
   (bvuge newA #x80) (ccV m) (ccI m) (ccD m) (ccC m) (= #x00 newA) (mem m) (exitLoc m)
  )
)

(define-fun set-sp ((new Byte) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) new
    (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccS ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    new (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccV ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) new (ccI m) (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccI ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) (ccV m) new (ccD m) (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccD ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) (ccV m) (ccI m) new (ccC m) (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccC ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) (ccV m) (ccI m) (ccD m) new (ccZ m) (mem m) (exitLoc m)
  )
)

(define-fun set-ccZ ((new Bool) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) new (mem m) (exitLoc m)
  )
)

(define-fun read-byte ((addr Word) (m Machine)) Byte
  (select (mem m) addr)
)

(define-fun write-byte ((addr Word) (val Byte) (m Machine)) Machine
  (mk-machine2 m (a m) (x m) (y m) (sp m)
    (ccS m) (ccV m) (ccI m) (ccD m) (ccC m) (ccZ m) (store (mem m) addr val) (exitLoc m)
  )
)

(define-fun indirect-x ((addr Byte) (m Machine)) Word
  (let ((newAddr (bvadd addr (x m))))
    (bvor
      (bvshl
        (ext (select (mem m) (ext (bvadd newAddr #x01))))
        #x0008)
      (ext (select (mem m) (ext newAddr))))))
      
(define-fun indirect-y ((addr Byte) (m Machine)) Word
  (bvadd (ext (y m)) 
    (bvor
      (bvshl
        (ext (select (mem m) (ext (bvadd addr #x01))))
        #x0008)
      (ext (select (mem m) (ext addr))))))
      
; todo jmp indirect bug
(define-fun indirect ((addr Word) (m Machine)) Word
    (bvor
      (bvshl
        (ext (select (mem m) (bvadd addr #x0001)))
        #x0008)
      (ext (select (mem m) addr))))
      
(define-fun absolute-x ((addr Word) (m Machine)) Word
  (bvadd (ext (x m)) addr)
)

(define-fun absolute-y ((addr Word) (m Machine)) Word
  (bvadd (ext (y m)) addr)
)

(define-fun zp-x ((addr Byte) (m Machine)) Word
  (ext (bvadd addr (x m)))
)

(define-fun zp-y ((addr Byte) (m Machine)) Word
  (ext (bvadd addr (y m)))
)

(define-fun zp ((addr Byte)) Word
  (ext addr)
)

(define-fun stack ((m Machine)) Word
  (bvadd #x0100 (ext (sp m)))
)

(declare-const aLive Bool)
(declare-const xLive Bool)
(declare-const yLive Bool)
(declare-const sLive Bool)
(declare-const zLive Bool)
(declare-const cLive Bool)
(declare-const vLive Bool)

(define-fun sameState ((m1 Machine) (m2 Machine)) Bool
  (and
   (= (mem m1) (mem m2))
   (= (exitLoc m1) (exitLoc m2))
   (=> aLive (= (a m1) (a m2)))
   (=> xLive (= (x m1) (x m2)))
   (=> yLive (= (y m1) (y m2)))
   (=> sLive (= (ccS m1) (ccS m2)))
   (=> vLive (= (ccV m1) (ccV m2)))
   (= (ccI m1) (ccI m2))
   (= (ccD m1) (ccD m2))
   (=> cLive (= (ccC m1) (ccC m2)))
   (=> zLive (= (ccZ m1) (ccZ m2)))
  )
)

(define-fun tax ((m Machine)) Machine
  (set-xSZ (a m) m)
)

(define-fun txa ((m Machine)) Machine
  (set-aSZ (x m) m)
)

(define-fun tay ((m Machine)) Machine
  (set-ySZ (a m) m)
)

(define-fun tya ((m Machine)) Machine
  (set-aSZ (y m) m)
)

(define-fun inx ((m Machine)) Machine
  (set-xSZ (bvadd (x m) #x01) m)
)

(define-fun dex ((m Machine)) Machine
  (set-xSZ (bvsub (x m) #x01) m)
)

(define-fun iny ((m Machine)) Machine
  (set-ySZ (bvadd (y m) #x01) m)
)

(define-fun dey ((m Machine)) Machine
  (set-ySZ (bvsub (y m) #x01) m)
)

(define-fun clc ((m Machine)) Machine
   (set-ccC false m)
)
 
(define-fun sec ((m Machine)) Machine
  (set-ccC true m)  
)

(define-fun cli ((m Machine)) Machine
  (set-ccI false m)
)

(define-fun sei ((m Machine)) Machine
  (set-ccI true m)
)

(define-fun clv ((m Machine)) Machine
  (set-ccV false m)
)

(define-fun cld ((m Machine)) Machine
  (set-ccD false m)
)

(define-fun sed ((m Machine)) Machine
  (set-ccD true m)
)

(define-fun tsx ((m Machine)) Machine
  (set-x (sp m) m)
)

(define-fun txs ((m Machine)) Machine
  (set-sp (x m) m)
)

(define-fun nop ((m Machine)) Machine
  m
)

(define-fun cpxImm ((imm Byte) (m Machine)) Machine
  (set-ccC
    (bvuge (x m) imm)
    (set-ccS
      (bvuge (bvsub (x m) imm) #x80)
      (set-ccZ (= (x m) imm) m))))

(define-fun cpxAbs ((addr Word) (m Machine)) Machine
  (cpxImm (read-byte addr m) m)
)

(define-fun cmpImm ((imm Byte) (m Machine)) Machine
  (set-ccC
    (bvuge (a m) imm)
    (set-ccS
      (bvuge (bvsub (a m) imm) #x80)
      (set-ccZ (= (a m) imm) m))))

(define-fun cmpAbs ((addr Word) (m Machine)) Machine
  (cmpImm (read-byte addr m) m)
)

(define-fun cpyImm ((imm Byte) (m Machine)) Machine
  (set-ccC
    (bvuge (y m) imm)
    (set-ccS
      (bvuge (bvsub (y m) imm) #x80)
      (set-ccZ (= (y m) imm) m))))

(define-fun cpyAbs ((addr Word) (m Machine)) Machine
  (cpyImm (read-byte addr m) m)
)

(define-fun andImm ((imm Byte) (m Machine)) Machine
  (set-aSZ (bvand imm (a m)) m)
)

(define-fun andAbs ((addr Word) (m Machine)) Machine
  (andImm (read-byte addr m) m)
)

(define-fun oraImm ((imm Byte) (m Machine)) Machine
  (set-aSZ (bvor imm (a m)) m)
)

(define-fun oraAbs ((addr Word) (m Machine)) Machine
  (oraImm (read-byte addr m) m)
)

(define-fun eorImm ((imm Byte) (m Machine)) Machine
  (set-aSZ (bvxor imm (a m)) m)
)

(define-fun eorAbs ((addr Word) (m Machine)) Machine
  (eorImm (read-byte addr m) m)
)

(define-fun ldaImm ((imm Byte) (m Machine)) Machine
  (set-aSZ imm m)
)

(define-fun ldaAbs ((addr Word) (m Machine)) Machine
  (ldaImm (read-byte addr m) m)
)

(define-fun ldaZp ((addr Byte) (m Machine)) Machine
  (ldaAbs (ext addr) m)
)

(define-fun ldaZpX ((addr Byte) (m Machine)) Machine
  (ldaAbs (zp-x addr m) m)
)

(define-fun ldaAbsX ((addr Word) (m Machine)) Machine
  (ldaAbs (absolute-x addr m) m)
)

(define-fun ldaAbsY ((addr Word) (m Machine)) Machine
  (ldaAbs (absolute-y addr m) m)
)

(define-fun ldaIndX ((addr Byte) (m Machine)) Machine
  (ldaAbs (indirect-x addr m) m)
)

(define-fun ldxImm ((imm Byte) (m Machine)) Machine
  (set-xSZ imm m)
)

(define-fun ldxAbs ((addr Word) (m Machine)) Machine
  (ldxImm (read-byte addr m) m)
)

(define-fun ldyImm ((imm Byte) (m Machine)) Machine
  (set-ySZ imm m)
)

(define-fun ldyAbs ((addr Word) (m Machine)) Machine
  (ldyImm (read-byte addr m) m)
)

(define-fun pla ((m Machine)) Machine
  (let ((newM (set-sp (bvadd #x01 (sp m)) m)))
    (set-a (read-byte (stack newM) newM) newM)
  ))
  
(define-fun pha ((m Machine)) Machine
  (set-sp (bvadd #x01 (sp m)) (write-byte (stack m) (a m) m))
)

(define-fun get-p ((m Machine)) Byte
  (bvor
    (ite (ccS m) #x80 #x00)
    (ite (ccV m) #x40 #x00)
    (ite (ccD m) #x08 #x00)
    (ite (ccI m) #x04 #x00)
    (ite (ccZ m) #x02 #x00)
    (ite (ccC m) #x01 #x00)
  )
)

(define-fun set-p ((p Byte) (m Machine)) Machine
  (set-ccS
    (= #x80 (bvand p #x80))
    (set-ccV
      (= #x40 (bvand p #x40))
      (set-ccD
        (= #x08 (bvand p #x08))
        (set-ccI
          (= #x04 (bvand p #x04))
          (set-ccZ
            (= #x02 (bvand p #x02))
              (set-ccC
                (= #x01 (bvand p #x01))
                m)))))))

(define-fun plp ((m Machine)) Machine
  (let ((newM (set-sp (bvadd #x01 (sp m)) m)))
    (set-p (read-byte (stack newM) newM) newM)
  ))

(define-fun php ((m Machine)) Machine
  (set-sp (bvadd #x01 (sp m)) (write-byte (stack m) (get-p m) m))
)

(define-fun staAbs ((addr Word) (m Machine)) Machine
  (write-byte addr (a m) m)
)

(define-fun stxAbs ((addr Word) (m Machine)) Machine
  (write-byte addr (x m) m)
)

(define-fun styAbs ((addr Word) (m Machine)) Machine
  (write-byte addr (y m) m)
)

(define-fun bitAbs ((addr Word) (m Machine)) Machine
  (let ((byte (read-byte addr m)))
    (set-ccZ
      (= #x00 (bvand (a m) byte))
      (set-ccS
        (= #x80 (bvand byte #x80))
        (set-ccV
          (= #x40 (bvand byte #x40))
          m)))))
          
(define-fun rts ((m Machine)) Machine
  (set-exit (Left Rts) m)
)

(define-fun rti ((m Machine)) Machine
  (set-exit (Left Rti) m)
)

(define-fun jmp ((addr Word) (m Machine)) Machine
  (set-exit (Right addr) m))
  
(define-fun jmpi ((addr Word) (m Machine)) Machine
  (set-exit (Right (indirect addr m)) m)
)

(define-fun beq ((addr Word) (m Machine)) Machine
  (ite (ccZ m)
    (set-exit (Right addr) m)
    m
  )
)
(define-fun bne ((addr Word) (m Machine)) Machine
  (ite (ccZ m)
    m
    (set-exit (Right addr) m)
  )
)

(define-fun bcs ((addr Word) (m Machine)) Machine
  (ite (ccC m)
    (set-exit (Right addr) m)
    m
  )
)
(define-fun bcc((addr Word) (m Machine)) Machine
  (ite (ccC m)
    m
    (set-exit (Right addr) m)
  )
)

(define-fun bvs ((addr Word) (m Machine)) Machine
  (ite (ccV m)
    (set-exit (Right addr) m)
    m
  )
)
(define-fun bvc ((addr Word) (m Machine)) Machine
  (ite (ccV m)
    m
    (set-exit (Right addr) m)
  )
)

(define-fun bmi ((addr Word) (m Machine)) Machine
  (ite (ccS m)
    (set-exit (Right addr) m)
    m
  )
)
(define-fun bpl ((addr Word) (m Machine)) Machine
  (ite (ccS m)
    m
    (set-exit (Right addr) m)
  )
)

(define-fun incAbs ((addr Word) (m Machine)) Machine
  (let ((newVal (bvadd #x01 (read-byte addr m))))
    (write-byte addr newVal (set-SZ newVal m))))

(define-fun decAbs ((addr Word) (m Machine)) Machine
  (let ((newVal (bvsub (read-byte addr m) #x01)))
    (write-byte addr newVal (set-SZ newVal m))))

(define-fun aslA ((m Machine)) Machine
  (set-ccC
    (= #x80 (bvand #x80 (a m)))
    (set-aSZ (bvshl (a m) #x01) m)
  )
)

(define-fun aslAbs ((addr Word) (m Machine)) Machine
  (let ((val (read-byte addr m)))
    (set-ccC
      (= #x80 (bvand #x80 val))
      (write-byte addr (bvshl val #x01) (set-SZ (bvshl val #x01) m))
    )
  )
)

(define-fun rolA ((m Machine)) Machine
  (let ((val (bvor (bvshl (a m) #x01) (ite (ccC m) #x01 #x00))))
    (set-ccC
      (= #x80 (bvand #x80 (a m)))
      (set-aSZ val m))))
      
(define-fun rolAbs ((addr Word) (m Machine)) Machine
  (let ((o_val (read-byte addr m)))
    (let ((val (bvor (bvshl o_val #x01) (ite (ccC m) #x01 #x00))))
      (set-ccC
        (= #x80 (bvand #x80 o_val))
        (write-byte addr val m)))))
        
(define-fun rorA ((m Machine)) Machine
  (let ((val (bvor (bvlshr (a m) #x01) (ite (ccC m) #x80 #x00))))
    (set-ccC
      (= #x01 (bvand #x01 (a m)))
      (set-aSZ val m))))
      
(define-fun rorAbs ((addr Word) (m Machine)) Machine
  (let ((o_val (read-byte addr m)))
    (let ((val (bvor (bvlshr o_val #x01) (ite (ccC m) #x80 #x00))))
      (set-ccC
        (= #x01 (bvand #x01 o_val))
        (write-byte addr val m)))))

(define-fun lsrA ((m Machine)) Machine
  (set-ccC
    (= #x01 (bvand #x01 (a m)))
    (set-aSZ (bvlshr (a m) #x01) m)
  )
)

(define-fun lsrAbs ((addr Word) (m Machine)) Machine
  (let ((val (read-byte addr m)))
    (set-ccC
      (= #x01 (bvand #x01 val))
      (write-byte addr (bvlshr val #x01) (set-SZ (bvlshr val #x01) m))
    )
  )
)

(define-fun adcImm ((imm Byte) (m Machine)) Machine
  (let ((sum (bvadd (ext imm) (ext (a m)) (ite (ccC m) #x0001 #x0000))))
    (set-ccV
      (= #x80 (bvand (bvxor (lobyte sum) (a m)) (bvxor (lobyte sum) imm) #x80))
      (set-ccC
        (= (hibyte sum) #x01)
        (set-aSZ (lobyte sum) m)) 
    )
  )
)

(define-fun adcAbs ((addr Word) (m Machine)) Machine
  (adcImm (read-byte addr m) m)
)

(define-fun sbcImm ((imm Byte) (m Machine)) Machine
  (adcImm (bvnot imm) m)
)

(define-fun sbcAbs ((addr Word) (m Machine)) Machine
  (sbcImm (read-byte addr m) m)
)

(declare-const someState Machine)
(assert (= (Left End) (exitLoc someState)))

(assert aLive)
(assert xLive)
(assert yLive)
(assert sLive)
(assert zLive)
(assert cLive)
(assert vLive)

