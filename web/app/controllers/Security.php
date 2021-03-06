<?php

/*
 * The Security Controller
 * Link http://github.com/typefo/lamb
 * Copyright (C) typefo <typefo@qq.com>
 */

class SecurityController extends Yaf\Controller_Abstract {
    public function blacklistAction() {
        $security = new SecurityModel();
        $this->getView()->assign('blacklist', $security->blacklist());
        return true;
    }

    public function whitelistAction() {
        $security = new SecurityModel();
        $this->getView()->assign('whitelist', $security->whitelist());
        return true;
    }

    public function deleteAction() {
        $request = $this->getRequest();

        $type = $request->getQuery('type', null);
        $security = new SecurityModel();
        switch ($type) {
        case 'blacklist':
            $security->delete('blacklist', $request->getQuery('phone'));
            break;
        case 'whitelist':
            $security->delete('whitelist', $request->getQuery('phone'));
            break;
        }
        
        return false;
    }

    public function importAction() {
        $module = 'blacklist';

        $request = $this->getRequest();
        if ($request->isPost() && isset($_FILES['file'])) {
            $security = new SecurityModel();
            $type = $request->getPost('type', null);

            switch ($type) {
            case 'blacklist':
                $module = 'blacklist';
                $security->import('blacklist', $_FILES['file']);
                break;
            case 'whitelist':
                $module = 'whitelist';
                $security->import('whitelist', $_FILES['file']);
                break;
            }
        }

        $url = 'http://' . $_SERVER['SERVER_ADDR'] . ':' . $_SERVER['SERVER_PORT'] . '/security/' . $module;
        $response = $this->getResponse();
        $response->setRedirect($url);
        $response->response();
        return false;
    }

    public function checkAction() {
        $sucesss = false;
        $request = $this->getRequest();
        $type = $request->getQuery('type', null);
        $security = new SecurityModel();

        switch ($type) {
        case 'blacklist':
            $success = $security->isExist('blacklist', $request->getQuery('phone'));
            break;
        case 'whitelist':
            $success = $security->isExist('whitelist', $request->getQuery('phone'));
            break;
        }

        $response['message'] = $success ? 'yes' : 'no';
        header('Content-type: application/json');
        echo json_encode($response);
        return false;
    }

    public function keywordAction() {
        return true;
    }
}
